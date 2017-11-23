//
//  TerrainCursorSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#ifndef TerrainCursorSystem_hpp
#define TerrainCursorSystem_hpp

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>
#include "Terrain/Terrain.hpp"

#include "TerrainCursor.hpp"
#include "ActiveCamera.hpp"
#include "KeypressEvent.hpp"
#include "MouseMoveEvent.hpp"

class TerrainCursorSystem : public entityx::System<TerrainCursorSystem>, public entityx::Receiver<TerrainCursorSystem>
{
public:
    TerrainCursorSystem();
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const KeypressEvent &event);
    void receive(const MouseMoveEvent &event);
    
private:
    void updateCursor(TerrainCursor &cursor,
                      const glm::mat4 &transform,
                      const std::shared_ptr<Terrain> &terrain);
    
    entityx::Entity _activeCamera;
    bool _needsUpdate;
};

#endif /* TerrainCursorSystem_hpp */
