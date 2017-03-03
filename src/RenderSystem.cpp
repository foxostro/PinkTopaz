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
#include "Exception.hpp"

#include "SDL.h"
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp> // for lookAt
#include <sstream>

namespace PinkTopaz {
    
    RenderSystem::RenderSystem(const std::shared_ptr<Renderer::GraphicsDevice> &dev)
     : _windowSizeChangeEventPending(false),
       _graphicsDevice(dev),
       _stringRenderer(dev, "vegur/Vegur-Regular.otf", 48),
       _timeAccum(0),
       _countDown(60),
       _framesBetweenReport(60),
       _firstReportingPeriod(true)
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
        unsigned ticksBeginMs = SDL_GetTicks();
        
        glm::mat4x4 cameraTransform;
        if (_activeCamera.valid()) {
            cameraTransform = _activeCamera.component<Transform>()->value;
        }
        
        std::set<std::shared_ptr<Renderer::Shader>> shadersEncountered;
        
        Renderer::RenderPassDescriptor desc; // default values
        auto encoder = _graphicsDevice->encoder(desc);
        
        encoder->setViewport(_viewport);

        auto f = [&](entityx::Entity entity,
                     RenderableStaticMesh &mesh,
                     Transform &transform) {
            encoder->setShader(mesh.shader);
            
            // If we have a new projection matrix then pass it to each shader used for rendering.
            // Take care to do this only once for each unique shader encountered.
            if (_windowSizeChangeEventPending) {
                if (shadersEncountered.find(mesh.shader) == shadersEncountered.end()) {
                    shadersEncountered.insert(mesh.shader);
                    mesh.shader->setShaderUniform("proj", _proj);
                }
            }
            
            mesh.shader->setShaderUniform("view", cameraTransform * transform.value);

            encoder->setFragmentSampler(mesh.textureSampler, 0);
            encoder->setFragmentTexture(mesh.texture, 0);
            encoder->setVertexBuffer(mesh.buffer);
            
            size_t count = mesh.buffer->getVertexCount();
            encoder->drawPrimitives(Renderer::Triangles, 0, count, 1);
        };
        es.each<RenderableStaticMesh, Transform>(f);
        
        _graphicsDevice->submit(encoder);
        
        // Draw text strings on the screen last because they blend.
        _stringRenderer.draw(_viewport);

        _graphicsDevice->swapBuffers();
        _windowSizeChangeEventPending = false;
        
        // Keep track of the time we spent rendering.
        unsigned ticksEndMs = SDL_GetTicks();
        unsigned ticksElapsedMs = ticksEndMs - ticksBeginMs;
        _timeAccum += ticksElapsedMs;
        
        if (_countDown == 0) {
            float frameTime = (float)_timeAccum / (float)_framesBetweenReport;
            
            std::stringstream ss;
            ss.precision(2);
            ss << std::fixed << frameTime;
            std::string s(ss.str());
            std::string string = "Frame Time: " + s + " ms";
        
            if (_firstReportingPeriod) {
                _firstReportingPeriod = false;
                const glm::vec3 color(0.2f, 0.2f, 0.2f);
                const glm::vec2 position(30.0f, 1140.0f);
                _frameTimeLabel = _stringRenderer.add(string, position, color);
            } else {
                _stringRenderer.replaceContents(_frameTimeLabel, string);
            }
            
            _countDown = _framesBetweenReport;
            _timeAccum = 0;
        } else {
            --_countDown;
        }
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
        constexpr float znear = 0.1f;
        constexpr float zfar = 100.0f;
        _viewport = glm::ivec4(0, 0, event.width * event.windowScaleFactor, event.height * event.windowScaleFactor);
        _proj = glm::perspective(glm::pi<float>() * 0.25f, (float)event.width / event.height, znear, zfar);
        _windowSizeChangeEventPending = true;
    }
    
} // namespace PinkTopaz
