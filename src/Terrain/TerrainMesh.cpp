//
//  TerrainMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainMesh.hpp"
#include <thread>

TerrainMesh::TerrainMesh(const AABB &meshBox,
                         const std::shared_ptr<RenderableStaticMesh> &defMesh,
                         const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                         const std::shared_ptr<Mesher> &mesher,
                         const std::shared_ptr<VoxelDataSource> &voxels)
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

RenderableStaticMesh TerrainMesh::getMesh() const
{
    std::lock_guard<std::mutex> lock(_lockMesh);
    return _mesh;
}

void TerrainMesh::rebuild(TerrainProgressTracker &progress)
{
    std::lock_guard<std::mutex> lock(_lockMeshInFlight);
    
    // We need a border of voxels around the region of the mesh in order to
    // perform surface extraction.
    const AABB voxelBox = _meshBox.inset(-_voxels->cellDimensions());
    
    progress.setState(TerrainProgressEvent::WaitingOnVoxels);
    _voxels->readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        progress.setState(TerrainProgressEvent::ExtractingSurface);
        
        StaticMesh mesh = _mesher->extract(voxels, _meshBox);
        
        std::shared_ptr<Buffer> vertexBuffer = nullptr;
        
        if (mesh.getVertexCount() > 0) {
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
