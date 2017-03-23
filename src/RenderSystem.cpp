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
#include <glm/gtc/matrix_transform.hpp> // for perspective()

namespace PinkTopaz {
    
    RenderSystem::RenderSystem(const std::shared_ptr<Renderer::GraphicsDevice> &dev)
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
        FRAME_TIMER(_frameTimer);
        
        glm::mat4x4 cameraTransform;
        if (_activeCamera.valid()) {
            cameraTransform = _activeCamera.component<Transform>()->value;
        }
        
        const glm::mat4 adjust = _graphicsDevice->getProjectionAdjustMatrix();
        
        // Update the uniform buffers so they include the most recent matrices.
        auto f = [&](entityx::Entity entity,
                     RenderableStaticMesh &mesh,
                     Transform &transform) {
            Renderer::TerrainUniforms uniforms = {
                .view = cameraTransform * transform.value,
                .proj = adjust * _proj,
            };
            mesh.uniforms->replace(sizeof(uniforms), &uniforms);
        };
        es.each<RenderableStaticMesh, Transform>(f);
        
        // Render all meshes.
        auto encoder = _graphicsDevice->encoder((Renderer::RenderPassDescriptor) {
            .clear = true,
            .clearColor = glm::vec4(0.2f, 0.4f, 0.5f, 1.0f)
        });
        
        encoder->setViewport(_viewport);
        auto g = [&](entityx::Entity entity,
                     RenderableStaticMesh &mesh,
                     Transform &transform) {
            encoder->setShader(mesh.shader);
            encoder->setFragmentSampler(mesh.textureSampler, 0);
            encoder->setFragmentTexture(mesh.texture, 0);
            encoder->setVertexBuffer(mesh.buffer, 0);
            encoder->setVertexBuffer(mesh.uniforms, 1);
            encoder->drawPrimitives(Renderer::Triangles, 0, mesh.vertexCount, 1);
        };
        es.each<RenderableStaticMesh, Transform>(g);
        
        // Draw text strings on the screen last because they blend.
        encoder->setDepthTest(false);
        _stringRenderer.draw(encoder, _viewport);
        
        encoder->commit();

        _graphicsDevice->swapBuffers();
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
        constexpr float zfar = 1000.0f;
        _viewport = glm::ivec4(0, 0, event.width * event.windowScaleFactor, event.height * event.windowScaleFactor);
        _proj = glm::perspective(glm::pi<float>() * 0.25f, (float)event.width / event.height, znear, zfar);
        _graphicsDevice->windowSizeChanged();
    }
    
} // namespace PinkTopaz
