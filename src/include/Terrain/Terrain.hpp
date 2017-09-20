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
#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/TerrainMesh.hpp"
#include "Terrain/TerrainDrawList.hpp"
#include "Terrain/TerrainChunkQueue.hpp"
#include "Terrain/TerrainMeshGrid.hpp"
#include "Terrain/TerrainHorizonDistance.hpp"
#include "Terrain/TerrainProgressTracker.hpp"
#include "RenderableStaticMesh.hpp"

// Object represents the voxel terrain of the world.
class Terrain
{
public:
    static constexpr unsigned TERRAIN_CHUNK_SIZE = 16;
    static constexpr float ACTIVE_REGION_SIZE = 256.0f;
    
    ~Terrain();
    
    Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
            const std::shared_ptr<TaskDispatcher> &dispatcher,
            const std::shared_ptr<TaskDispatcher> &dispatcherRebuildMesh);
    
    // No default constructor.
    Terrain() = delete;
    
    // Updates terrain over time.
    void update(entityx::TimeDelta dt);
    
    // Passes uniforms down to all terrain meshes.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws the portions of the terrain which are in view.
    void draw(const std::shared_ptr<CommandEncoder> &encoder);
    
    // The terrain system changes the fog density to hide the horizon of loading
    // terrain chunks.
    float getFogDensity() const;
    
private:
    using MaybeTerrainMesh = boost::optional<TerrainMesh>;
    
    inline AABB getActiveRegion() const
    {
        const glm::vec3 cameraPos = _cameraPosition;
        const float horizonDistance = _horizonDistance.get();
        const AABB horizonBox = {cameraPos, glm::vec3(horizonDistance, horizonDistance, horizonDistance)};
        const AABB activeRegion = _meshes->boundingBox().intersect(horizonBox);
        return activeRegion;
    }
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<TaskDispatcher> _dispatcherRebuildMesh;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataGenerator> _voxelDataGenerator;
    std::shared_ptr<VoxelDataStore> _voxels;
    std::unique_ptr<TerrainDrawList> _drawList;
    std::unique_ptr<TerrainMeshGrid> _meshes;
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    TerrainChunkQueue _meshesToRebuild;
    glm::mat4x4 _modelViewProjection;
    std::atomic<glm::vec3> _cameraPosition;
    TerrainHorizonDistance _horizonDistance;
    TerrainProgressTracker _progressTracker;
    
    void fetchMeshes(const std::vector<AABB> &meshCells);
    
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    // Kicks off asynchronous tasks to rebuild any meshes that are affected by
    // the specified changes.
    void rebuildMeshInResponseToChanges(const ChangeLog &changeLog);
    
    // Rebuilds the next pending mesh in the queue.
    void rebuildNextMesh();
};

#endif /* Terrain_hpp */
