//
//  TerrainMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainMesh.hpp"
#include <thread>

TerrainMesh::~TerrainMesh() {}

TerrainMesh::TerrainMesh(const AABB &meshBox,
                         const std::shared_ptr<RenderableStaticMesh> &defMesh,
                         const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                         const std::shared_ptr<Mesher> &mesher,
                         const std::shared_ptr<VoxelDataStore> &voxels)
 : _graphicsDevice(graphicsDevice),
   _mesher(mesher),
   _voxels(voxels),
   _defaultMesh(defMesh),
   _mesh(*defMesh),
   _meshBox(meshBox)
{}

TerrainMesh::TerrainMesh(const TerrainMesh &mesh)
 : _graphicsDevice(mesh._graphicsDevice),
   _mesher(mesh._mesher),
   _voxels(mesh._voxels),
   _defaultMesh(mesh._defaultMesh),
   _mesh(mesh._mesh),
   _meshBox(mesh._meshBox)
{}

TerrainMesh::TerrainMesh(TerrainMesh &&mesh)
 : _graphicsDevice(mesh._graphicsDevice),
   _mesher(mesh._mesher),
   _voxels(mesh._voxels),
   _defaultMesh(mesh._defaultMesh),
   _mesh(mesh._mesh),
   _meshBox(mesh._meshBox)
{}

TerrainMesh& TerrainMesh::operator=(const TerrainMesh &rhs)
{
    if (this == &rhs) {
        return *this;
    }
    
    _graphicsDevice = rhs._graphicsDevice;
    _mesher = rhs._mesher;
    _voxels = rhs._voxels;
    _defaultMesh = rhs._defaultMesh;
    _mesh = rhs._mesh;
    _meshBox = rhs._meshBox;
    
    return *this;
}

TerrainMesh::MaybeMesh TerrainMesh::nonblockingGetMesh() const
{
    MaybeMesh mesh;
    
    if (_lockMesh.try_lock()) {
        mesh.emplace(_mesh);
        _lockMesh.unlock();
    }
    
    return mesh;
}

void TerrainMesh::rebuild()
{
    std::lock_guard<std::mutex> lock(_lockMeshInFlight);
    
    // We need a border of voxels around the region of the mesh in order to
    // perform surface extraction.
    const AABB voxelBox = _meshBox.inset(-_voxels->cellDimensions());
    
    _voxels->readerTransaction(voxelBox, [&](const GridAddressable<Voxel> &voxels){
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
        
        {
            std::lock_guard<std::mutex> lock(_lockMesh);
            _mesh = renderableStaticMesh;
        }
    });
}
