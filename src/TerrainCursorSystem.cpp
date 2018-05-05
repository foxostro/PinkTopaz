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

TerrainCursorSystem::TerrainCursorSystem(const std::shared_ptr<TaskDispatcher> &dispatcher,
                                         const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher)
 : _dispatcher(dispatcher),
   _mainThreadDispatcher(mainThreadDispatcher),
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
        
        es.each<TerrainCursor,
                RenderableStaticWireframeMesh,
                Transform>([&](entityx::Entity cursorEntity,
                               TerrainCursor &cursor,
                               RenderableStaticWireframeMesh &cursorMesh,
                               Transform &cursorTransform) {
            
            entityx::Entity terrainEntity = cursor.terrainEntity;
            
            const Transform *terrainTransformPtr = terrainEntity.component<Transform>().get();
            const Transform &terrainTransform = *terrainTransformPtr;
            
            const TerrainComponent *terrainPtr = terrainEntity.component<TerrainComponent>().get();
            const TerrainComponent &terrain = *terrainPtr;
            
            const glm::mat4 cameraTerrainTransform = cameraTransform * terrainTransform.value;
            requestCursorUpdate(cursor,
                                cursorTransform,
                                cursorMesh,
                                cameraTerrainTransform,
                                terrain.terrain);
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
                                              Transform &cursorTransform,
                                              RenderableStaticWireframeMesh &cursorMesh,
                                              const glm::mat4 &cameraTerrainTransform,
                                              const std::shared_ptr<Terrain> &t)
{
    using namespace glm;
    
    // Cancel a pending cursor update, if there is one.
    if (cursor.cancellationToken) {
        cursor.cancellationToken->store(true);
    }

    // TODO: It would help here to allocate these tokens from an object pool.
    cursor.cancellationToken = std::make_shared<std::atomic<bool>>(false);

    // Schedule a task to asynchronously compute the updated cursor position.
    _dispatcher->async([startTime=std::chrono::steady_clock::now(),
                        cameraTerrainTransform,
                        terrain=t,
                        cancellationToken=cursor.cancellationToken]{

        if (cancellationToken && cancellationToken->load()) {
            throw BrokenPromiseException();
        }
        
        const vec3 cameraEye(inverse(cameraTerrainTransform)[3]);
        const quat cameraOrientation = toQuat(transpose(cameraTerrainTransform));
        const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
        const Ray ray(cameraEye, rayDir);
        const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
        const auto &voxels = terrain->getVoxels();
        
        bool active = false;
        glm::vec3 cursorPos, placePos;
        
        voxels.readerTransaction(voxelBox, [&](const Array3D<Voxel> &voxels){
            for (const auto &pos : slice(voxels, ray, maxPlaceDistance)) {
                const Voxel &voxel = voxels.reference(pos);
                
                if (voxel.value != 0.f) {
                    active = true;
                    cursorPos = pos;
                    break;
                } else {
                    placePos = pos;
                }
            }
        });
        
        // Return a tuple containing the updated cursor value and the start time
        // of the computation.
        return std::make_tuple(active, cursorPos, placePos, startTime);

    }).then(_mainThreadDispatcher, [cancellationToken=cursor.cancellationToken,
                                    &cursor,
                                    &cursorTransform,
                                    &cursorMesh](auto tuple){

        // This task executes on the main thread because we'll be using it to
        // update our entity's components.
        //
        // TODO: If the entity is destroyed before this task is executed then
        //       the references we captured will be invalid. We should instead
        //       capture a handle to the entity itself.

        const auto& [active, cursorPos, placePos, startTime] = tuple;

        cursor.active = active;
        cursor.pos = cursorPos;
        cursor.placePos = placePos;

        cursorTransform.value = glm::translate(mat4(1), placePos + vec3(0.5f));
        
        cursorMesh.hidden = !active;

//        const auto currentTime = std::chrono::steady_clock::now();
//        const auto duration = currentTime - startTime;
//        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
//        const std::string msStr = std::to_string(ms.count());
//        SDL_Log("Got new cursor value in %s milliseconds", msStr.c_str());
    });
}
