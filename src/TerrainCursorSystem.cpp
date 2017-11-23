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

static constexpr size_t maxPlaceDistance = 4;

TerrainCursorSystem::TerrainCursorSystem()
 : _needsUpdate(false)
{}

void TerrainCursorSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    em.subscribe<KeypressEvent>(*this);
    em.subscribe<MouseMoveEvent>(*this);
}

void TerrainCursorSystem::update(entityx::EntityManager &es,
                                 entityx::EventManager &events,
                                 entityx::TimeDelta deltaMilliseconds)
{
    if (!_activeCamera.valid()) {
        return;
    }
    
    if (!_needsUpdate) {
        return;
    }
    
    const auto &cameraTransform = _activeCamera.component<Transform>()->value;
    
    es.each<TerrainCursor, Transform, TerrainComponent>([&](entityx::Entity terrainEntity,
                                                            TerrainCursor &cursor,
                                                            Transform &terrainTransform,
                                                            TerrainComponent &terrain) {
        
        const glm::mat4 transform = cameraTransform * terrainTransform.value;
        updateCursor(cursor, transform, terrain.terrain);
    });
    
    _needsUpdate = false;
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

void TerrainCursorSystem::receive(const KeypressEvent &event)
{
    _needsUpdate = true;
}

void TerrainCursorSystem::receive(const MouseMoveEvent &event)
{
    _needsUpdate = true;
}

void TerrainCursorSystem::updateCursor(TerrainCursor &cursor,
                                       const glm::mat4 &transform,
                                       const std::shared_ptr<Terrain> &terrain)
{
    using namespace glm;
    
    const vec3 cameraEye(inverse(transform)[3]);
    const quat cameraOrientation = toQuat(transpose(transform));
    const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
    const Ray ray(cameraEye, rayDir);
    const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
    const auto &voxels = terrain->getVoxels();
    
//    SDL_Log("ray: origin=(%.2f, %.2f, %.2f) ; direction=(%.2f, %.2f, %.2f)",
//            ray.origin.x, ray.origin.y, ray.origin.z,
//            ray.direction.x, ray.direction.y, ray.direction.z);
//
//    SDL_Log("voxelBox: center=(%.2f, %.2f, %.2f) ; extent=(%.2f, %.2f, %.2f)",
//            voxelBox.center.x, voxelBox.center.y, voxelBox.center.z,
//            voxelBox.extent.x, voxelBox.extent.y, voxelBox.extent.z);
    
    bool cursorIsActive = false;
    vec3 prev = ray.origin;
    vec3 cursorPos;
    
    voxels.readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
        for (const auto pos : slice(voxels, ray, maxPlaceDistance)) {
            const Voxel &voxel = voxels.reference(pos);
            
            if (voxel.value != 0.f) {
                cursorIsActive = true;
                cursorPos = pos;
                break;
            } else {
                prev = pos;
            }
        }
    });
    
    cursor.active = cursorIsActive;
    cursor.pos = cursorPos;
    cursor.placePos = prev;
    
    if (cursor.active) {
        SDL_Log("cursorIsActive: %s", cursorIsActive ? "true" : "false");
        SDL_Log("cursorPos: (%.2f, %.2f, %.2f)", cursorPos.x, cursorPos.y, cursorPos.z);
        SDL_Log("prev: (%.2f, %.2f, %.2f)", prev.x, prev.y, prev.z);
    }
}
