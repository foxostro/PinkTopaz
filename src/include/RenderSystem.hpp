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
#include <boost/filesystem.hpp>

#include "ActiveCamera.hpp"
#include "TerrainComponent.hpp"
#include "WindowSizeChangedEvent.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/TextRenderer.hpp"
#include "FrameTimer.hpp"

// System for rendering game objects in the world.
class RenderSystem : public entityx::System<RenderSystem>, public entityx::Receiver<RenderSystem>
{
public:
    RenderSystem(const std::shared_ptr<GraphicsDevice> &graphicsDevice);
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const WindowSizeChangedEvent &event);
        
private:
    const boost::filesystem::path FONT_NAME;
    const unsigned FONT_SIZE;
    
    glm::ivec4 _viewport;
    glm::mat4x4 _proj;
    entityx::Entity _terrainEntity;
    entityx::Entity _activeCamera;
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    TextRenderer _stringRenderer;
    FrameTimer _frameTimer;
};

#endif /* RenderSystem_hpp */
