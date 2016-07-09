//
//  RenderSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef RenderSystem_hpp
#define RenderSystem_hpp

#include <glm/mat4x4.hpp>
#include <entityx/entityx.h>

#include "ActiveCamera.hpp"
#include "WindowSizeChangedEvent.hpp"

namespace PinkTopaz {
    
    // System for rendering game objects in the world.
    class RenderSystem : public entityx::System<RenderSystem>, public entityx::Receiver<RenderSystem>
    {
    public:
        RenderSystem();
        void configure(entityx::EventManager &em) override;
        void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
        void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
        void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
        void receive(const WindowSizeChangedEvent &event);
        
    private:
        bool _projHasBeenUpdated;
        glm::mat4x4 _proj;
        entityx::Entity _activeCamera;
    };

} // namespace PinkTopaz

#endif /* RenderSystem_hpp */
