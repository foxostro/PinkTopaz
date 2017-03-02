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
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/StringRenderer.hpp"

namespace PinkTopaz {
    
    // System for rendering game objects in the world.
    class RenderSystem : public entityx::System<RenderSystem>, public entityx::Receiver<RenderSystem>
    {
    public:
        RenderSystem(const std::shared_ptr<Renderer::GraphicsDevice> &graphicsDevice);
        void configure(entityx::EventManager &em) override;
        void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
        void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
        void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
        void receive(const WindowSizeChangedEvent &event);
        
    private:
        bool _windowSizeChangeEventPending;
        glm::ivec4 _viewport;
        glm::mat4x4 _proj;
        entityx::Entity _activeCamera;
        std::shared_ptr<Renderer::GraphicsDevice> _graphicsDevice;
        Renderer::StringRenderer _stringRenderer;
        Renderer::StringRenderer::StringHandle _fps;
        unsigned _timeAccum;
        unsigned _countDown;
        const unsigned _framesBetweenReport;
    };

} // namespace PinkTopaz

#endif /* RenderSystem_hpp */
