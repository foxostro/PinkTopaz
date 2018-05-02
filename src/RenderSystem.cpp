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
#include "Fonts/TextAttributes.hpp"
#include "Transform.hpp"
#include "RenderableStaticMesh.hpp"
#include "TerrainComponent.hpp"
#include "TerrainCursor.hpp"
#include "Exception.hpp"
#include "Profiler.hpp"

#include "SDL.h"
#include <glm/gtc/matrix_transform.hpp> // for perspective()

RenderSystem::RenderSystem(const std::shared_ptr<GraphicsDevice> &dev)
 : _graphicsDevice(dev),
   _textRenderer(dev, TextAttributes(
       /*fontName=*/    "Vegur",
       /*fontSize=*/    24,
       /*border=*/      1,
       /*color=*/       glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
       /*borderColor=*/ glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
       /*weight=*/      Bold
   )),
   _frameTimer(_textRenderer)
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
    glm::mat4x4 cameraTransform;
    if (_activeCamera.valid()) {
        cameraTransform = _activeCamera.component<Transform>()->value;
    }
    
    // Update the uniform buffers so they include the most recent matrices.
    const glm::mat4 adjust = _graphicsDevice->getProjectionAdjustMatrix();
    
    float fogDensity = 0.0f;
    es.each<TerrainComponent, Transform>([&](entityx::Entity entity,
                                             TerrainComponent &terrain,
                                             Transform &transform) {
        terrain.terrain->update(dt);
        fogDensity = terrain.terrain->getFogDensity();
        
        TerrainUniforms uniforms = {
            cameraTransform * transform.value,
            adjust * _proj,
            fogDensity
        };
        terrain.terrain->setTerrainUniforms(uniforms);
    });
    
    es.each<RenderableStaticMesh, Transform>([&](entityx::Entity entity,
                                                 RenderableStaticMesh &mesh,
                                                 Transform &transform) {
        TerrainUniforms uniforms = {
            cameraTransform * transform.value,
            adjust * _proj,
            fogDensity
        };
        mesh.uniforms->replace(sizeof(uniforms), &uniforms);
    });
    
    // Render all meshes.
    RenderPassDescriptor desc = {
        /* clear = */ true,
        /* clear color = */ glm::vec4(0.2f, 0.4f, 0.5f, 1.0f)
    };
    auto encoder = _graphicsDevice->encoder(desc);
    
    encoder->setViewport(_viewport);
    
    es.each<TerrainComponent>([&](entityx::Entity entity, TerrainComponent &terrain){
        terrain.terrain->draw(encoder);
    });
    
    // Draw static meshes.
    es.each<RenderableStaticMesh>([&](entityx::Entity entity, RenderableStaticMesh &mesh){
        encoder->setShader(mesh.shader);
        encoder->setFragmentSampler(mesh.textureSampler, 0);
        encoder->setFragmentTexture(mesh.texture, 0);
        encoder->setVertexBuffer(mesh.buffer, 0);
        encoder->setVertexBuffer(mesh.uniforms, 1);
        encoder->drawPrimitives(Triangles, 0, mesh.vertexCount, 1);
    });
    
    // TODO: Draw terrain cursors.
    es.each<TerrainCursor>([&](entityx::Entity terrainEntity,
                               TerrainCursor &cursor) {
        if (cursor.active) {
            SDL_Log("RenderSystem: active: %s", cursor.active ? "true" : "false");
            SDL_Log("RenderSystem: pos: (%.2f, %.2f, %.2f)", cursor.pos.x, cursor.pos.y, cursor.pos.z);
            SDL_Log("RenderSystem: placePos: (%.2f, %.2f, %.2f)", cursor.placePos.x, cursor.placePos.y, cursor.placePos.z);
        }
    });
    
    // Draw text strings on the screen last because they blend.
    encoder->setDepthTest(false);
    _textRenderer.draw(encoder, _viewport);
    
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
    _textRenderer.setWindowScaleFactor(event.windowScaleFactor);
    _graphicsDevice->windowSizeChanged();
}
