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
    // If the camera has moved then we need to request a new terrain cursor.
    if (_activeCamera.valid() && _needsUpdate) {
        _needsUpdate = false;
        
        const auto &cameraTransform = _activeCamera.component<Transform>()->value;
        
        es.each<TerrainCursor, Transform, TerrainComponent>([&](entityx::Entity terrainEntity,
                                                                TerrainCursor &cursor,
                                                                Transform &terrainTransform,
                                                                TerrainComponent &terrain) {
            
            const glm::mat4 transform = cameraTransform * terrainTransform.value;
            requestCursorUpdate(cursor, transform, terrain.terrain);
        });
    }
    
    // Retrieve asynchronously calculated results when they are ready.
    es.each<TerrainCursor>([&](entityx::Entity terrainEntity,
                               TerrainCursor &cursor) {
        pollPendingCursorUpdate(cursor);
    });
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
                                              const glm::mat4 &transform,
                                              const std::shared_ptr<Terrain> &t)
{
    // Cancel any currently pending cursor request.
    if (cursor.pending) {
        cursor.pending->cancel();
    }
    
    const auto startTime = std::chrono::steady_clock::now();
    
    auto future = _dispatcher->async([this,
                                      startTime,
                                      transform,
                                      terrain=t]{
        const auto maybeValue = calcCursor(transform, terrain);
        return std::make_tuple(maybeValue, startTime);
    });
    cursor.pending.emplace(std::move(future));
}

void TerrainCursorSystem::pollPendingCursorUpdate(TerrainCursor &cursor)
{
    if (!cursor.pending) {
        return;
    }
    
    auto &future = *cursor.pending;
    
    if (future.isReady()) {
        const auto& [value, startTime] = future.get();
    
        cursor.value = value;
        
        const auto currentTime = std::chrono::steady_clock::now();
        const auto duration = currentTime - startTime;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        const std::string msStr = std::to_string(ms.count());
        SDL_Log("Got new cursor value in %s milliseconds", msStr.c_str());
        
        cursor.pending.reset();
    }
}

TerrainCursorValue
TerrainCursorSystem::calcCursor(const glm::mat4 &transform,
                                const std::shared_ptr<Terrain> &terrain)
{
    using namespace glm;
    
    const vec3 cameraEye(inverse(transform)[3]);
    const quat cameraOrientation = toQuat(transpose(transform));
    const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
    const Ray ray(cameraEye, rayDir);
    const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
    const auto &voxels = terrain->getVoxels();
    
    TerrainCursorValue cursor;
    
    cursor.active = false;
    
    voxels.readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        for (const auto pos : slice(voxels, ray, maxPlaceDistance)) {
            const Voxel &voxel = voxels.reference(pos);
            
            if (voxel.value != 0.f) {
                cursor.active = true;
                cursor.pos = pos;
                break;
            } else {
                cursor.placePos = pos;
            }
        }
    });
    
    return cursor;
}
