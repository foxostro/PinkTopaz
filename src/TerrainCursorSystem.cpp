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

void TerrainCursorSystem::update(entityx::EntityManager &es,
                                 entityx::EventManager &events,
                                 entityx::TimeDelta deltaMilliseconds)
{
    es.each<ActiveCamera, Transform>([&](entityx::Entity cameraEntity,
                                         ActiveCamera &activeCamera,
                                         Transform &cameraTransform) {
        es.each<TerrainCursor, Transform, TerrainComponent>([&](entityx::Entity terrainEntity,
                                                                TerrainCursor &cursor,
                                                                Transform &terrainTransform,
                                                                TerrainComponent &terrain) {
            updateCursor(cursor, cameraTransform, terrainTransform, terrain);
        });
    });
}

void TerrainCursorSystem::updateCursor(TerrainCursor &cursor,
                                       const Transform &cameraTransform,
                                       const Transform &terrainTransform,
                                       const TerrainComponent &terrain)
{
    using namespace glm;
    
    const mat4 transform = cameraTransform.value * terrainTransform.value;
    const vec3 cameraEye(inverse(transform)[3]);
    const quat cameraOrientation = toQuat(transpose(transform));
    const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
    const Ray ray(cameraEye, rayDir);
    const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
    const auto &voxels = terrain.terrain->getVoxels();
    
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
}
