//
//  World.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include "World.hpp"
#include "Transform.hpp"
#include "ActiveCamera.hpp"
#include "RenderableStaticMesh.hpp"
#include "RenderSystem.hpp"

namespace PinkTopaz {
    
    World::World(const std::shared_ptr<Renderer::GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<Renderer::Buffer> &buffer,
                 const std::shared_ptr<Renderer::Shader> &shader,
                 const std::shared_ptr<Renderer::Texture> &texture)
    {
        systems.add<RenderSystem>(graphicsDevice);
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
        terrain.assign<RenderableStaticMesh>(buffer, shader, texture);
        terrain.assign<Transform>(glm::mat4x4());
    }
    
    void World::update(entityx::TimeDelta dt)
    {
        systems.update_all(dt);
    }
    
} // namespace PinkTopaz
