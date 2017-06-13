//
//  Terrain.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Terrain_hpp
#define Terrain_hpp

#include "Renderer/GraphicsDevice.hpp"
#include "TaskDispatcher.hpp"
#include "Terrain/Mesher.hpp"
#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/TerrainMesh.hpp"
#include "Terrain/TerrainDrawList.hpp"
#include "Terrain/TerrainMeshQueue.hpp"
#include "RenderableStaticMesh.hpp"
#include <experimental/optional>

class Terrain
{
public:
    ~Terrain();
    
    Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
            const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // No default constructor.
    Terrain() = delete;
    
    // Passes uniforms down to all terrain meshes.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws the portions of the terrain which are in view.
    void draw(const std::shared_ptr<CommandEncoder> &encoder);
    
private:
    typedef typename std::experimental::optional<TerrainMesh> MaybeTerrainMesh;
    
    static constexpr int MESH_CHUNK_SIZE = 16;
    
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataStore> _voxels;
    std::unique_ptr<TerrainDrawList> _drawList;
    std::mutex _lockMeshes;
    std::unique_ptr<Array3D<MaybeTerrainMesh>> _meshes;
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    TerrainMeshQueue _meshesToRebuild;
    std::atomic<int> _meshFetchInFlight;
    
    // Kicks off asynchronous tasks to rebuild any meshes that are affected by
    // the specified changes.
    void asyncRebuildMeshes(const ChangeLog &changeLog);
    
    // Kicks off an asynchronous task to rebuild the mesh at the specified cell.
    void asyncRebuildAnotherMesh(const AABB &cell);
    
    // Examines `_meshesToRebuild' and rebuilds the best mesh, if any meshes
    // are pending.
    void rebuildNextMesh();
};

#endif /* Terrain_hpp */
