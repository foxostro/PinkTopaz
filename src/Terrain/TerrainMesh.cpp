//
//  TerrainMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainMesh.hpp"

TerrainMesh::~TerrainMesh() {}

TerrainMesh::TerrainMesh(const AABB &meshBox,
                         const std::shared_ptr<RenderableStaticMesh> &defaultMesh,
                         const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                         const std::shared_ptr<Mesher> &mesher,
                         const std::shared_ptr<VoxelDataStore> &voxels)
 : _graphicsDevice(graphicsDevice),
   _mesher(mesher),
   _voxels(voxels),
   _defaultMesh(defaultMesh),
   _mesh(*defaultMesh),
   _meshBox(meshBox)
{
    rebuild();
}

void TerrainMesh::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    // The uniforms referenced in the default mesh are also referenced by other
    // meshes in the terrain. Setting it once here sets it for all of them.
    _defaultMesh->uniforms->replace(sizeof(uniforms), &uniforms);
}

void TerrainMesh::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    // Several resources are set before this call and are reused for all meshes.
    // We expect the caller to take care of that for us.
    
    if (_mesh.vertexCount > 0) {
        encoder->setVertexBuffer(_mesh.buffer, 0);
        encoder->drawPrimitives(Triangles, 0, _mesh.vertexCount, 1);
    }
}

void TerrainMesh::rebuild()
{
    // We need a border of voxels around the region of the mesh in order to
    // perform surface extraction.
    const AABB voxelBox = _meshBox.inset(-glm::vec3(1, 1, 1));
    
    _voxels->readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        // The voxel file uses a binary SOLID/EMPTY flag for voxels.
        // So, we get values that are either 0.0 or 1.0.
        constexpr float isosurface = 0.5f;
        
        StaticMesh mesh = _mesher->extract(voxels, _meshBox, isosurface);
        
        std::shared_ptr<Buffer> vertexBuffer = nullptr;
        
        if (mesh.getVertexCount() > 0) {
            // AFOX_TODO: We may need to create graphics resources on the main thread, e.g., when using OpenGL.
            auto vertexBufferData = mesh.getBufferData();
            vertexBuffer = _graphicsDevice->makeBuffer(vertexBufferData,
                                                       StaticDraw,
                                                       ArrayBuffer);
            vertexBuffer->addDebugMarker("Terrain Vertices", 0, vertexBufferData.size());
        }
        
        RenderableStaticMesh renderableStaticMesh = *_defaultMesh;
        renderableStaticMesh.vertexCount = mesh.getVertexCount();
        renderableStaticMesh.buffer = vertexBuffer;
        
        // AFOX_TODO: Do I need to expose the updated mesh atomically?
        _mesh = renderableStaticMesh;
    });
}
