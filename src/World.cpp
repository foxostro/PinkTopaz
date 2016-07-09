//
//  World.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // for lookAt

#include "World.hpp"

namespace PinkTopaz {

    // Tags the entity which acts as the camera.
    // We expect there to only be one entity at a time that has the ActiveCamera component. Systems will generally
    // listen for the event where this component is added to an entity and set that entity as the camera.
    struct ActiveCamera
    {
        ActiveCamera() {}
    };
    
    // Position and Orientation of an entity.
    struct Transform
    {
        Transform() {}
        
        Transform(const glm::mat4x4 &val) : value(val) {}
        
        Transform(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
        {
            value = glm::lookAt(eye, center, up);
        }
        
        glm::mat4x4 value;
    };
    
    // Gives the entity a static mesh which is rendered to represent the entity in the world.
    struct RenderableStaticMesh
    {
        RenderableStaticMesh() {}
        
        RenderableStaticMesh(const std::shared_ptr<StaticMeshVAO> &vao,
                             const std::shared_ptr<Shader> &shader,
                             const std::shared_ptr<TextureArray> &texture)
        {
            this->vao = vao;
            this->shader = shader;
            this->texture = texture;
        }
        
        std::shared_ptr<StaticMeshVAO> vao;
        std::shared_ptr<Shader> shader;
        std::shared_ptr<TextureArray> texture;
    };
    
    // System for rendering static meshes in the world, associated with the RenderableStaticMesh components.
    class StaticMeshRenderSystem : public entityx::System<StaticMeshRenderSystem>,
                                   public entityx::Receiver<StaticMeshRenderSystem>
    {
    public:
        StaticMeshRenderSystem() {}
        
        void configure(entityx::EventManager &em) override
        {
            em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
            em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
        }
        
        void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override
        {
            glm::mat4x4 cameraTransform;
            if (_activeCamera.valid()) {
                cameraTransform = _activeCamera.component<Transform>()->value;
            }
            
            auto f = [cameraTransform](entityx::Entity entity, RenderableStaticMesh &mesh, Transform &transform) {
                mesh.shader->bind();
                mesh.shader->setUniform("view", cameraTransform * transform.value);
                mesh.texture->bind();
                glBindVertexArray(mesh.vao->getVAO());
                glDrawArrays(GL_TRIANGLES, 0, mesh.vao->getNumVerts());
                mesh.shader->unbind();
            };
            
            es.each<RenderableStaticMesh, Transform>(f);
        }
        
        void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event)
        {
            _activeCamera = event.entity;
        }
        
        void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event)
        {
            if (_activeCamera == event.entity) {
                _activeCamera.invalidate();
            }
        }
        
    private:
        entityx::Entity _activeCamera;
    };
    
    World::World(const std::shared_ptr<StaticMeshVAO> &vao,
                 const std::shared_ptr<Shader> &shader,
                 const std::shared_ptr<TextureArray> &texture)
    {
        systems.add<StaticMeshRenderSystem>();
        systems.configure();
        
        // Create an entity to represent the camera.
        // Render systems will know by the ActiveCamera that this is the camera. They will retrieve the entity's
        // transformation and take it into account when rendering their stuff.
        entityx::Entity camera = entities.create();
        camera.assign<Transform>(glm::vec3(85.1, 16.1, 140.1),
                                 glm::vec3(80.1, 20.1, 130.1),
                                 glm::vec3(0, 1, 0));
        camera.assign<ActiveCamera>();
        
        // Create an entity to represent the terrain.
        entityx::Entity terrain = entities.create();
        terrain.assign<RenderableStaticMesh>(vao, shader, texture);
        terrain.assign<Transform>(glm::mat4x4());
    }
    
    void World::update(entityx::TimeDelta dt)
    {
        systems.update_all(dt);
    }
    
} // namespace PinkTopaz
