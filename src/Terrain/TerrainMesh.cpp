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
                         const std::shared_ptr<Mesher> &mesher)
 : _graphicsDevice(graphicsDevice),
   _mesher(mesher),
   _defaultMesh(defMesh),
   _mesh(*defMesh),
   _meshBox(meshBox)
{}

TerrainMesh::TerrainMesh(const TerrainMesh &mesh)
 : _graphicsDevice(mesh._graphicsDevice),
   _mesher(mesh._mesher),
   _defaultMesh(mesh._defaultMesh),
   _mesh(mesh._mesh),
   _meshBox(mesh._meshBox)
{}

TerrainMesh::TerrainMesh(TerrainMesh &&mesh)
 : _graphicsDevice(mesh._graphicsDevice),
   _mesher(mesh._mesher),
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
    _defaultMesh = rhs._defaultMesh;
    _mesh = rhs._mesh;
    _meshBox = rhs._meshBox;
    
    return *this;
}

RenderableStaticMesh TerrainMesh::getMesh() const
{
    std::scoped_lock lock(_lockMesh);
    return _mesh;
}

void TerrainMesh::rebuild(const Array3D<Voxel> &voxels, TerrainProgressTracker &progress)
{
    std::scoped_lock lock(_lockMeshInFlight);
    
    progress.setState(TerrainProgressEvent::ExtractingSurface);
    
    StaticMesh mesh = _mesher->extract(voxels, _meshBox);
    
    std::shared_ptr<Buffer> vertexBuffer = nullptr;
    
    if (mesh.getVertexCount() > 0) {
        auto [size, data] = mesh.getBufferData();
        vertexBuffer = _graphicsDevice->makeBuffer(size, data,
                                                   StaticDraw, ArrayBuffer);
        vertexBuffer->addDebugMarker("Terrain Vertices", 0, size);
    }
    
    RenderableStaticMesh renderableStaticMesh = *_defaultMesh;
    renderableStaticMesh.vertexCount = mesh.getVertexCount();
    renderableStaticMesh.buffer = vertexBuffer;
    
    {
        std::scoped_lock lock(_lockMesh);
        _mesh = renderableStaticMesh;
    }
}
