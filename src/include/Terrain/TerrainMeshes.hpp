//
//  TerrainMeshes.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainMeshes_hpp
#define TerrainMeshes_hpp

#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/Mesher.hpp"
#include "Terrain/VoxelDataGenerator.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"
#include "TaskDispatcher.hpp"

// Terrain is broken up into several meshes.
class TerrainMeshes
{
public:
    ~TerrainMeshes();
    
    TerrainMeshes(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                  const std::shared_ptr<TaskDispatcher> &dispatcher,
                  const std::shared_ptr<VoxelDataStore> &voxels);
    
    // No default constructor.
    TerrainMeshes() = delete;
    
    // Passes uniforms down to all terrain meshes.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws the portions of the terrain which are in view.
    void draw(const std::shared_ptr<CommandEncoder> &encoder) const;
    
private:
    static constexpr int MESH_CHUNK_SIZE = 16;
    
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::unique_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataStore> _voxels;
    
    mutable std::mutex _lockMeshes;
    std::unique_ptr<Array3D<RenderableStaticMesh>> _meshes;
    RenderableStaticMesh _defaultMesh;
    
    void rebuildMesh(const ChangeLog &changeLog);
};

#endif /* TerrainMeshes_hpp */
