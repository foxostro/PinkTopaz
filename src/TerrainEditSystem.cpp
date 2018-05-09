//
//  TerrainEditSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#include "TerrainEditSystem.hpp"
#include "TerrainCursor.hpp"
#include "TerrainComponent.hpp"

TerrainEditSystem::TerrainEditSystem() = default;

void TerrainEditSystem::configure(entityx::EventManager &eventManager)
{
    eventManager.subscribe<MouseButtonEvent>(*this);
}

void TerrainEditSystem::update(entityx::EntityManager &es,
                               entityx::EventManager &events,
                               entityx::TimeDelta deltaMilliseconds)
{
    while (!_pendingEvents.empty()) {
        MouseButtonEvent event = _pendingEvents.front();
        _pendingEvents.pop();
        
        if (event.button == SDL_BUTTON_LEFT) {
            if (event.down) {
                SDL_Log("SDL_BUTTON_LEFT pressed");
            } else {
                SDL_Log("SDL_BUTTON_LEFT released");
            }
        }
    } // while there are pending events
}

void TerrainEditSystem::receive(const MouseButtonEvent &event)
{
    _pendingEvents.push(event);
}
