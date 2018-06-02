//
//  Terrain.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Terrain_hpp
#define Terrain_hpp

#include "Preferences.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "TaskDispatcher.hpp"
#include "Terrain/Mesher.hpp"
#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/TransactedVoxelData.hpp"
#include "Terrain/TerrainMesh.hpp"
#include "Terrain/TerrainRebuildActor.hpp"
#include "Terrain/TerrainMeshGrid.hpp"
#include "Terrain/TerrainHorizonDistance.hpp"
#include "Terrain/TerrainConfig.hpp"
#include "Terrain/TerrainJournal.hpp"
#include "RenderableStaticMesh.hpp"

#include <entityx/entityx.h>
#include <memory>
#include <spdlog/spdlog.h>

// Object represents the voxel terrain of the world.
class Terrain
{
public:
    ~Terrain();
    
    Terrain(const Preferences &preferences,
            std::shared_ptr<spdlog::logger> log,
            const std::shared_ptr<GraphicsDevice> &graphicsDevice,
            const std::shared_ptr<TaskDispatcher> &dispatcher,
            const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData,
            const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher,
            entityx::EventManager &events,
            glm::vec3 initialCameraPosition);
    
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
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, std::function<void(Array3D<Voxel> &&data)> fn);
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(const std::shared_ptr<TerrainOperation> &operation);
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<TransactedVoxelData> _voxels;
    std::unique_ptr<TerrainMeshGrid> _meshes;
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    std::unique_ptr<TerrainRebuildActor> _meshRebuildActor;
    std::unique_ptr<TerrainJournal> _journal;
    glm::mat4x4 _modelViewProjection;
    std::atomic<glm::vec3> _cameraPosition;
    TerrainHorizonDistance _horizonDistance;
    std::shared_ptr<spdlog::logger> _log;
    const float _activeRegionSize;
    std::chrono::steady_clock::time_point _startTime;
    
    inline AABB getActiveRegion() const
    {
        const glm::vec3 cameraPos = _cameraPosition;
        const float horizonDistance = _horizonDistance.get();
        const AABB horizonBox = {cameraPos, glm::vec3(horizonDistance, horizonDistance, horizonDistance)};
        const AABB activeRegion = _meshes->boundingBox().intersect(horizonBox);
        return activeRegion;
    }
    
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    // Kicks off asynchronous tasks to rebuild any meshes in a region affected
    // by a change.
    void rebuildMeshInResponseToChanges(const AABB &affectedRegion);
    
    // Rebuilds the next pending mesh in the queue.
    void rebuildNextMesh(const AABB &cell, TerrainProgressTracker &progress);
    
    std::unique_ptr<TransactedVoxelData>
    createVoxelData(const std::shared_ptr<TaskDispatcher> &dispatcher,
                    unsigned voxelDataSeed,
                    const boost::filesystem::path &mapDirectory);
};

#endif /* Terrain_hpp */
