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
#include "Terrain/TerrainMeshQueue.hpp"
#include "RenderableStaticMesh.hpp"
#include <entityx/entityx.h> // for TimeDelta
#include <experimental/optional>
#include <shared_mutex>
#include <algorithm>

// The terrain horizon scrolls away from the camera as chunks are loaded.
class TerrainHorizonDistance
{
public:
    TerrainHorizonDistance()
     : _targetHorizonDistance(STEP),
       _horizonDistance(STEP)
    {}
    
    // Gets the horizon distance. Takes the lock so this is thread-safe.
    float get() const
    {
        std::shared_lock<std::shared_mutex> lock(_lockHorizonDistance);
        return _horizonDistance;
    }
    
    // Increments the horizon distance by the step.
    // Takes the lock so this is thread-safe.
    void increment()
    {
        std::unique_lock<std::shared_mutex> lock(_lockHorizonDistance);
        _targetHorizonDistance += STEP;
    }
    
    // The horizon distance moves away smoothly over time.
    void update(entityx::TimeDelta dt)
    {
        std::unique_lock<std::shared_mutex> lock(_lockHorizonDistance);
        _horizonDistance = std::min((float)(_horizonDistance + dt * STEP_PER_MS), _targetHorizonDistance);
    }
    
private:
    static constexpr float STEP = 16.0f;
    static constexpr float STEP_PER_MS = STEP / 1000.0f;
    mutable std::shared_mutex _lockHorizonDistance;
    float _targetHorizonDistance;
    float _horizonDistance;
};

// Object represents the voxel terrain of the world.
class Terrain
{
public:
    static constexpr unsigned TERRAIN_CHUNK_SIZE = 16;
    
    ~Terrain() = default;
    
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
    using MaybeTerrainMesh = typename std::experimental::optional<TerrainMesh>;
    
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<TaskDispatcher> _dispatcherRebuildMesh;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataGenerator> _voxelDataGenerator;
    std::shared_ptr<VoxelDataStore> _voxels;
    std::unique_ptr<TerrainDrawList> _drawList;
    std::unique_ptr<ConcurrentGridMutable<MaybeTerrainMesh>> _meshes;
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    TerrainMeshQueue _meshesToRebuild;
    glm::mat4x4 _modelViewProjection;
    std::atomic<glm::vec3> _cameraPosition;
    TerrainHorizonDistance _horizonDistance;
    
    // Kicks off asynchronous tasks to rebuild any meshes that are affected by
    // the specified changes.
    void rebuildMeshInResponseToChanges(const ChangeLog &changeLog);
    
    // Rebuilds the next pending mesh in the queue.
    void rebuildNextMesh();
};

#endif /* Terrain_hpp */
