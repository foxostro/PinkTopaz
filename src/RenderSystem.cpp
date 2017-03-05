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

constexpr const char *FONT_NAME = "vegur/Vegur-Regular.otf";
constexpr unsigned FONT_SIZE = 48;

namespace PinkTopaz {
    
    RenderSystem::RenderSystem(const std::shared_ptr<Renderer::GraphicsDevice> &dev)
     : _graphicsDevice(dev),
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
        _frameTimer.beginFrame();
        
        glm::mat4x4 cameraTransform;
        if (_activeCamera.valid()) {
            cameraTransform = _activeCamera.component<Transform>()->value;
        }
        
#if 0
        // Update the uniform buffers so they include the most recent matrices.
        auto f = [&](entityx::Entity entity,
                     RenderableStaticMesh &mesh,
                     Transform &transform) {
            Renderer::StaticMesh::Uniforms uniforms = {
                .view = cameraTransform * transform.value,
                .proj = _proj,
            };
            mesh.uniforms->replace(sizeof(uniforms), &uniforms);
        };
        es.each<RenderableStaticMesh, Transform>(f);
        
        // Render all meshes.
        auto g = [&](entityx::Entity entity,
                     RenderableStaticMesh &mesh,
                     Transform &transform) {
            // TODO: Probably want a single render pass for all such meshes...
            Renderer::RenderPassDescriptor desc = {
                .blend = false,
                .depthTest = true,
                .clear = true
            };
            auto encoder = _graphicsDevice->encoder(desc);
            encoder->setViewport(_viewport);
            encoder->setShader(mesh.shader);
            encoder->setFragmentSampler(mesh.textureSampler, 0);
            encoder->setFragmentTexture(mesh.texture, 0);
            encoder->setVertexBuffer(mesh.buffer, 0);
            encoder->setVertexBuffer(mesh.uniforms, 1);
            encoder->drawPrimitives(Renderer::Triangles, 0, mesh.vertexCount, 1);
            _graphicsDevice->submit(encoder);
        };
        es.each<RenderableStaticMesh, Transform>(g);
#endif
        
        // Draw text strings on the screen last because they blend.
        _stringRenderer.draw(_viewport);

        _frameTimer.endFrame();
        _graphicsDevice->swapBuffers();
        _frameTimer.afterFrame();
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
        _graphicsDevice->windowSizeChanged();
    }
    
} // namespace PinkTopaz
