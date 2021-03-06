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
#include "Terrain/TerrainOperationEditPoint.hpp"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::translate()
#include <mutex>

TerrainCursorSystem::TerrainCursorSystem(std::shared_ptr<spdlog::logger> log,
                                         const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher)
 : _dispatcher(std::make_shared<TaskDispatcher>("TerrainCursorSystem", 1)),
   _mainThreadDispatcher(mainThreadDispatcher),
   _needsUpdate(false),
   _mouseDownCounter(0),
   _log(log)
{}

void TerrainCursorSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    em.subscribe<CameraMovedEvent>(*this);
    em.subscribe<MouseButtonEvent>(*this);
}

void TerrainCursorSystem::update(entityx::EntityManager &es,
                                 entityx::EventManager &events,
                                 entityx::TimeDelta deltaMilliseconds)
{
    bool validActiveCamera = _activeCamera.valid();
    
    // If the camera has moved then we need to update the terrain cursor.
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
    
    // Process mouse button events which may affect the terrain cursor.
    while (!_pendingEvents.empty()) {
        MouseButtonEvent event = _pendingEvents.front();
        _pendingEvents.pop();
        
        // Change the terrain cursor color when the mouse button is depressed.
        _mouseDownCounter += (event.down ? 1 : -1);
        es.each<TerrainCursor, WireframeCube::Renderable>([&](entityx::Entity cursorEntity, TerrainCursor &cursor, WireframeCube::Renderable &cursorMesh) {
            if (_mouseDownCounter != 0) {
                cursorMesh.color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
            } else {
                cursorMesh.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        });
        
        if (event.button == SDL_BUTTON_LEFT && !event.down) {
            es.each<TerrainCursor>([&](entityx::Entity cursorEntity, TerrainCursor &cursor) {
                setBlockUnderCursor(cursor, events, /* value = */ true, /* usePlacePos = */ true);
            });
        }
        
        if (event.button == SDL_BUTTON_RIGHT && !event.down) {
            es.each<TerrainCursor>([&](entityx::Entity cursorEntity, TerrainCursor &cursor) {
                setBlockUnderCursor(cursor, events, /* value = */ false, /* usePlacePos = */ false);
            });
        }
    } // while there are pending events
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

void TerrainCursorSystem::receive(const MouseButtonEvent &event)
{
    _pendingEvents.push(event);
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
                        cancellationToken=cursor.cancellationToken,
                        log=_log]{

        if (cancellationToken && cancellationToken->load()) {
            throw BrokenPromiseException();
        }
        
        const vec3 cameraEye(inverse(cameraTerrainTransform)[3]);
        const quat cameraOrientation = toQuat(transpose(cameraTerrainTransform));
        const vec3 rayDir = cameraOrientation * vec3(0, 0, -1);
        const Ray ray(cameraEye, rayDir);
        const AABB voxelBox{cameraEye, vec3(maxPlaceDistance+1)};
        
        bool active = false;
        glm::vec3 cursorPos, placePos;
        
        try {
            terrain->readerTransaction(voxelBox, [&](Array3D<Voxel> &&voxels){
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
        } catch (OutOfBoundsException e) {
            log->error("TerrainCursorSystem::requestCursorUpdate triggered OutOfBoundsException: {}", e.what());
            throw BrokenPromiseException();
        }
        
        // Return a tuple containing the updated cursor value and the start time
        // of the computation.
        return std::make_tuple(active, cursorPos, placePos, startTime);

    }).then(_mainThreadDispatcher, [log=_log,
                                    cancellationToken=cursor.cancellationToken,
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

#ifdef SPDLOG_TRACE_ON
        const auto currentTime = std::chrono::steady_clock::now();
        const auto duration = currentTime - startTime;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        const std::string msStr = std::to_string(ms.count());
        log->trace("Got new cursor value in {} ms.", msStr);
#endif
    });
}

void TerrainCursorSystem::setBlockUnderCursor(TerrainCursor &cursor,
                                              entityx::EventManager &events,
                                              bool value,
                                              bool usePlacePos)
{
    if (!cursor.active) {
        return;
    }
    
    glm::vec3 location = usePlacePos ? cursor.placePos : cursor.pos;
    Voxel voxel{value};
    entityx::Entity terrainEntity = cursor.terrainEntity;
    
    if (!terrainEntity.valid()) {
        return;
    }
    
    auto handleTerrain = terrainEntity.component<TerrainComponent>();
    
    if (handleTerrain.valid()) {
        _needsUpdate = true;
        
        // Schedule a task to asynchronously update the terrain.
        // We do this off the main thread because it may block on a lock to
        // access the terrain.
        auto operation = std::make_shared<TerrainOperationEditPoint>(location, voxel);
        std::shared_ptr<Terrain> terrain = handleTerrain.get()->terrain;
        _dispatcher->async([terrain, operation]{
            terrain->writerTransaction(operation);
        });
    }
}
