//
//  TerrainEditSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#ifndef TerrainEditSystem_hpp
#define TerrainEditSystem_hpp

#include <entityx/entityx.h>
#include <queue>
#include "MouseButtonEvent.hpp"

// System for performing edits on Terrain at the cursor position.
class TerrainEditSystem : public entityx::System<TerrainEditSystem>, public entityx::Receiver<TerrainEditSystem>
{
public:
    TerrainEditSystem();
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es,
                entityx::EventManager &events,
                entityx::TimeDelta dt) override;
    void receive(const MouseButtonEvent &event);
    
private:
    std::queue<MouseButtonEvent> _pendingEvents;
};

#endif /* TerrainEditSystem_hpp */
