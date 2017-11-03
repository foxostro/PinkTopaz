//
//  RenderSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include "RenderSystem.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/RenderPassDescriptor.hpp"
#include "Transform.hpp"
#include "RenderableStaticMesh.hpp"
#include "TerrainComponent.hpp"
#include "Exception.hpp"
#include "Profiler.hpp"

#include "SDL.h"
#include <glm/gtc/matrix_transform.hpp> // for perspective()

RenderSystem::RenderSystem(const std::shared_ptr<GraphicsDevice> &dev)
 : FONT_NAME("vegur/Vegur-Regular.otf"),
   FONT_SIZE(48),
   _graphicsDevice(dev),
   _stringRenderer(dev, FONT_NAME, FONT_SIZE),
   _frameTimer(_stringRenderer)
{}

void RenderSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    em.subscribe<WindowSizeChangedEvent>(*this);
}
    
void RenderSystem::update(entityx::EntityManager &es,
                          entityx::EventManager &events,
                          entityx::TimeDelta dt)
{
    RenderPassDescriptor desc = {
        /* clear = */ true,
        /* clear color = */ glm::vec4(0.2f, 0.4f, 0.5f, 1.0f)
    };
    auto encoder = _graphicsDevice->encoder(desc);
    
    encoder->setViewport(_viewport);
    
    // Draw text strings on the screen.
    encoder->setDepthTest(false);
    _stringRenderer.draw(encoder, _viewport);
    
    encoder->commit();
    
    _graphicsDevice->swapBuffers();
    _frameTimer.tick();
}

void RenderSystem::receive(const entityx::ComponentAddedEvent<ActiveCamera> &event)
{
    _activeCamera = event.entity;
}
    
void RenderSystem::receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event)
{
    if (_activeCamera == event.entity) {
        _activeCamera.invalidate();
    }
}
    
void RenderSystem::receive(const WindowSizeChangedEvent &event)
{
    // When the window size changes, recalculate the projection matrix.
    // On the next update, we will pass this matrix to the shaders used to render each object.
    constexpr float znear = 1.0f;
    constexpr float zfar = 256.0f;
    _viewport = glm::ivec4(0, 0, event.width * event.windowScaleFactor, event.height * event.windowScaleFactor);
    _proj = glm::perspective(glm::pi<float>() * 0.25f, (float)event.width / event.height, znear, zfar);
    _graphicsDevice->windowSizeChanged();
}
