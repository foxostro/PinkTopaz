//
//  TerrainCursorSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#include "TerrainCursorSystem.hpp"
#include "TerrainCursor.hpp"
#include "ActiveCamera.hpp"
#include "Transform.hpp"
#include "TerrainComponent.hpp"
#include "Grid/GridRaycast.hpp"

#include "SDL.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::translate()
#include <mutex>

TerrainCursorSystem::TerrainCursorSystem(const std::shared_ptr<TaskDispatcher> &dispatcher)
 : _dispatcher(dispatcher),
   _needsUpdate(false)
{}

void TerrainCursorSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    em.subscribe<CameraMovedEvent>(*this);
}

void TerrainCursorSystem::update(entityx::EntityManager &es,
                                 entityx::EventManager &events,
                                 entityx::TimeDelta deltaMilliseconds)
{
    bool validActiveCamera = _activeCamera.valid();
    
    // If the camera has moved then we need to request a new terrain cursor.
    if (validActiveCamera && _needsUpdate) {
        _needsUpdate = false;
        
        const auto &cameraTransform = _activeCamera.component<Transform>()->value;
        
        es.each<TerrainCursor>([&](entityx::Entity cursorEntity,
                                   TerrainCursor &cursor) {
            
            entityx::Entity terrainEntity;
            {
                std::lock_guard<std::mutex> lock(cursor.lockValue);
                terrainEntity = cursor.value.terrainEntity;
            }
            
            const Transform *terrainTransformPtr = terrainEntity.component<Transform>().get();
            const Transform &terrainTransform = *terrainTransformPtr;
            
            const TerrainComponent *terrainPtr = terrainEntity.component<TerrainComponent>().get();
            const TerrainComponent &terrain = *terrainPtr;
            
            const glm::mat4 cameraTerrainTransform = cameraTransform * terrainTransform.value;
            requestCursorUpdate(cursor,
                                cameraTerrainTransform,
                                terrainEntity,
                                terrain.terrain);
        });
    }
    
    if (validActiveCamera) {
        // TODO: This could probably be made more efficient if we could signal
        // when the async voxel march is complete. As it is, we basically poll
        // every frame for the updated position of the cursor.
        es.each<TerrainCursor, Transform>([&](entityx::Entity cursorEntity,
                                              TerrainCursor &cursor,
                                              Transform &cursorTransform) {
            
            std::lock_guard<std::mutex> lock(cursor.lockValue);
            cursorTransform.value = glm::translate(glm::mat4(1), -cursor.value.pos);
        });
    }
}

void TerrainCursorSystem::receive(const entityx::ComponentAddedEvent<ActiveCamera> &event)
{
    _activeCamera = event.entity;
    _needsUpdate = true;
}

void TerrainCursorSystem::receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event)
{
    if (_activeCamera == event.entity) {
        _activeCamera.invalidate();
        _needsUpdate = false;
    }
}

void TerrainCursorSystem::receive(const CameraMovedEvent &event)
{
    _needsUpdate = true;
}

void TerrainCursorSystem::requestCursorUpdate(TerrainCursor &cursor,
                                              const glm::mat4 &cameraTerrainTransform,
                                              entityx::Entity terrainEntity,
                                              const std::shared_ptr<Terrain> &t)
{
    // Cancel a pending cursor update, if there is one.
    cursor.canceller();
    
    // Schedule a task to asynchronously compute the updated cursor position.
    auto future = _dispatcher->async([startTime=std::chrono::steady_clock::now(),
                                      cameraTerrainTransform,
                                      terrainEntity,
                                      terrain=t]{
        using namespace glm;
        
        const vec3 cameraEye(inverse(cameraTerrainTransform)[3]);
        const quat cameraOrientation = toQuat(transpose(cameraTerrainTransform));
        const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
        const Ray ray(cameraEye, rayDir);
        const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
        const auto &voxels = terrain->getVoxels();
        
        TerrainCursorValue value;
        
        value.active = false;
        value.terrainEntity = terrainEntity;
        
        voxels.readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
            for (const auto &pos : slice(voxels, ray, maxPlaceDistance)) {
                const Voxel &voxel = voxels.reference(pos);
                
                if (voxel.value != 0.f) {
                    value.active = true;
                    value.pos = pos;
                    break;
                } else {
                    value.placePos = pos;
                }
            }
        });
        
        // Return a tuple containing the updated cursor value and the start time
        // of the computation.
        return std::make_tuple(value, startTime);
    }).then([&cursor](auto tuple){
        // ...and now unpack that cursor value and start time.
        const auto& [value, startTime] = tuple;
        
        {
            std::lock_guard<std::mutex> lock(cursor.lockValue);
            cursor.value = value;
        }
        
        const auto currentTime = std::chrono::steady_clock::now();
        const auto duration = currentTime - startTime;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        const std::string msStr = std::to_string(ms.count());
        SDL_Log("Got new cursor value in %s milliseconds", msStr.c_str());
    });
    
    cursor.canceller = future.getCanceller();
}
