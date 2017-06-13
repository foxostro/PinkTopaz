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
#include "RenderableStaticMesh.hpp"
#include <experimental/optional>

// Terrain is broken up into several meshes.
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
    std::mutex _lockDrawList;
    std::unique_ptr<Array3D<RenderableStaticMesh>> _drawList;
    std::mutex _lockMeshes;
    std::unique_ptr<Array3D<MaybeTerrainMesh>> _meshes;
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    
    // Kicks off asynchronous tasks to rebuild any meshes that are affected by
    // the specified changes.
    void asyncRebuildMeshes(const ChangeLog &changeLog);
    
    // Rebuilds the one mesh associated with the specified cell.
    void rebuildMesh(const AABB &cell);
    
    // If we can get a hold of the underlying GPU resources then add them to the
    // draw list.
    void tryUpdateDrawList(const MaybeTerrainMesh &maybeTerrainMesh,
                           const AABB &cell);
};

#endif /* Terrain_hpp */
