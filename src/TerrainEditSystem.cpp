//
//  TerrainEditSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#include "TerrainEditSystem.hpp"
#include "TerrainCursorInvalidatedEvent.hpp"
#include "TerrainCursor.hpp"
#include "TerrainComponent.hpp"
#include "Terrain/TerrainOperationEditPoint.hpp"
#include "WireframeCube.hpp"

TerrainEditSystem::TerrainEditSystem()
 : _mouseDownCounter(0)
{}

void TerrainEditSystem::configure(entityx::EventManager &eventManager)
{
    eventManager.subscribe<MouseButtonEvent>(*this);
}

void TerrainEditSystem::setBlockUnderCursor(TerrainCursor &cursor,
                                            entityx::EventManager &events,
                                            float value,
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
        TerrainOperationEditPoint operation{location, voxel};
        auto terrain = handleTerrain.get()->terrain;
        terrain->writerTransaction(operation);
        
        TerrainCursorInvalidatedEvent event;
        events.emit(event);
    }
}

void TerrainEditSystem::update(entityx::EntityManager &es,
                               entityx::EventManager &events,
                               entityx::TimeDelta deltaMilliseconds)
{
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
                setBlockUnderCursor(cursor, events, /* value = */ 1.f, /* usePlacePos = */ true);
            });
        }
        
        if (event.button == SDL_BUTTON_RIGHT && !event.down) {
            es.each<TerrainCursor>([&](entityx::Entity cursorEntity, TerrainCursor &cursor) {
                setBlockUnderCursor(cursor, events, /* value = */ 0.f, /* usePlacePos = */ false);
            });
        }
    } // while there are pending events
}

void TerrainEditSystem::receive(const MouseButtonEvent &event)
{
    _pendingEvents.push(event);
}
