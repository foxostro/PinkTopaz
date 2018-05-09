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

TerrainEditSystem::TerrainEditSystem() = default;

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
