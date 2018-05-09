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
#include "WireframeCube.hpp"

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
    em.subscribe<TerrainCursorInvalidatedEvent>(*this);
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
        
        es.each<TerrainCursor, WireframeCube::Renderable, Transform>([&](entityx::Entity cursorEntity, TerrainCursor &cursor, WireframeCube::Renderable &cursorMesh, Transform &cursorTransform) {
            
            // The terrain entity, or its components, may be invalid. Check.
            entityx::Entity terrainEntity = cursor.terrainEntity;
            if (terrainEntity.valid()) {
                auto handleTransform = terrainEntity.component<Transform>();
                auto handleTerrain = terrainEntity.component<TerrainComponent>();
                
                if (handleTransform.valid() && handleTerrain.valid()) {
                    const Transform &terrainTransform = *handleTransform.get();
                    const TerrainComponent &terrain = *handleTerrain.get();
                    
                    const glm::mat4 cameraTerrainTransform = cameraTransform * terrainTransform.value;
                    requestCursorUpdate(cameraTerrainTransform,
                                        terrain.terrain,
                                        cursorEntity);
                }
            }
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

void TerrainCursorSystem::receive(const TerrainCursorInvalidatedEvent &event)
{
    _needsUpdate = true;
}

void TerrainCursorSystem::requestCursorUpdate(const glm::mat4 &cameraTerrainTransform,
                                              const std::shared_ptr<Terrain> &terrain,
                                              entityx::Entity cursorEntity)
{
    using namespace glm;
    
    // `cursor' must not be captured in the lambdas below, as the component
    // could be removed or destroyed in the interim.
    TerrainCursor &cursor = *cursorEntity.component<TerrainCursor>().get();
    
    // Cancel a pending cursor update, if there is one.
    if (cursor.cancellationToken) {
        cursor.cancellationToken->store(true);
    }

    cursor.cancellationToken = std::make_shared<std::atomic<bool>>(false);

    // Schedule a task to asynchronously compute the updated cursor position.
    _dispatcher->async([startTime=std::chrono::steady_clock::now(),
                        cameraTerrainTransform,
                        terrain=terrain,
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
                                    cursorEntity=std::move(cursorEntity)](auto tuple) mutable{

        // This task executes on the main thread because we'll be using it to
        // update our entity's components.

        if (cancellationToken && cancellationToken->load()) {
            throw BrokenPromiseException();
        }
        
        const auto& [active, cursorPos, placePos, startTime] = tuple;
        
        // The entity or these components may have been destroyed in the
        // interim. Check first.
        if (cursorEntity.valid()) {
            auto handleTerrainCursor = cursorEntity.component<TerrainCursor>();
            auto handleTransform = cursorEntity.component<Transform>();
            auto handleMesh = cursorEntity.component<WireframeCube::Renderable>();
            
            if (handleTerrainCursor.valid() &&
                handleTransform.valid() &&
                handleMesh.valid()) {
                
                TerrainCursor &cursor = *handleTerrainCursor.get();
                cursor.active = active;
                cursor.pos = cursorPos;
                cursor.placePos = placePos;
                
                Transform &cursorTransform = *handleTransform.get();
                cursorTransform.value = glm::translate(mat4(1), placePos + vec3(0.5f));
                
                WireframeCube::Renderable &cursorMesh = *handleMesh.get();
                cursorMesh.hidden = !active;
            }
        }

//        const auto currentTime = std::chrono::steady_clock::now();
//        const auto duration = currentTime - startTime;
//        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
//        const std::string msStr = std::to_string(ms.count());
//        SDL_Log("Got new cursor value in %s milliseconds", msStr.c_str());
    });
}
