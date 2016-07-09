//
//  RenderSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include "RenderSystem.hpp"
#include "Shader.hpp"
#include "Transform.hpp"
#include "RenderableStaticMesh.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp> // for lookAt

namespace PinkTopaz {
    
    RenderSystem::RenderSystem() {}
    
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
        
        std::set<std::shared_ptr<Shader>> shadersEncountered;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto f = [&](entityx::Entity entity, RenderableStaticMesh &mesh, Transform &transform) {
            mesh.shader->bind();
            
            // If we have a new projection matrix then pass it to each shader used for rendering.
            // Take care to do this only once for each unique shader encountered.
            if (_projHasBeenUpdated) {
                if (shadersEncountered.find(mesh.shader) == shadersEncountered.end()) {
                    shadersEncountered.insert(mesh.shader);
                    mesh.shader->setUniform("proj", _proj);
                }
            }
            
            mesh.shader->setUniform("view", cameraTransform * transform.value);
            mesh.texture->bind();
            glBindVertexArray(mesh.vao->getVAO());
            glDrawArrays(GL_TRIANGLES, 0, mesh.vao->getNumVerts());
            mesh.shader->unbind();
        };
        es.each<RenderableStaticMesh, Transform>(f);
        glFlush();
        
        _projHasBeenUpdated = false;
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
        glViewport(0, 0, event.width * event.windowScaleFactor, event.height * event.windowScaleFactor);
        _proj = glm::perspective(glm::pi<float>() * 0.25f, (float)event.width / event.height, znear, zfar);
        _projHasBeenUpdated = true;
    }
    
} // namespace PinkTopaz
