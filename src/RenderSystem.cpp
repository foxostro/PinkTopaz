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
#include "Transform.hpp"
#include "RenderableStaticMesh.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp> // for lookAt

namespace PinkTopaz {
    
    RenderSystem::RenderSystem(const std::shared_ptr<Renderer::GraphicsDevice> &graphicsDevice)
     : _windowSizeChangeEventPending(false),
       _graphicsDevice(graphicsDevice)
    {}
    
    void RenderSystem::configure(entityx::EventManager &em)
    {
        em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
        em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
        em.subscribe<WindowSizeChangedEvent>(*this);
    }
    
    void RenderSystem::update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt)
    {
        glm::mat4x4 cameraTransform;
        if (_activeCamera.valid()) {
            cameraTransform = _activeCamera.component<Transform>()->value;
        }
        
        std::set<std::shared_ptr<Renderer::Shader>> shadersEncountered;
        
        auto encoder = _graphicsDevice->encoder();
        
        if (_windowSizeChangeEventPending) {
            encoder->setViewport(_viewport);
        }

        auto f = [&](entityx::Entity entity, RenderableStaticMesh &mesh, Transform &transform) {
            encoder->setShader(mesh.shader); // TODO: Sort by shader to minimize the number of shader switches.
            
            // If we have a new projection matrix then pass it to each shader used for rendering.
            // Take care to do this only once for each unique shader encountered.
            if (_windowSizeChangeEventPending) {
                if (shadersEncountered.find(mesh.shader) == shadersEncountered.end()) {
                    shadersEncountered.insert(mesh.shader);
                    mesh.shader->setShaderUniform("proj", _proj);
                }
            }
            
            mesh.shader->setShaderUniform("view", cameraTransform * transform.value);
            encoder->setTexture(mesh.texture);
            encoder->setVertexArray(mesh.vao);
            encoder->drawTriangles(0, mesh.vao->getNumVerts());
        };
        es.each<RenderableStaticMesh, Transform>(f);
        
        _graphicsDevice->submit(encoder);
        _graphicsDevice->swapBuffers();
        
        _windowSizeChangeEventPending = false;
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
