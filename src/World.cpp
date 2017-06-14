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
#include "RenderSystem.hpp"
#include "CameraMovementSystem.hpp"
#include "TerrainComponent.hpp"
#include "Profiler.hpp"

#include <glm/gtc/matrix_transform.hpp>
    
World::World(const std::shared_ptr<GraphicsDevice> &device,
             const std::shared_ptr<TaskDispatcher> &dispatcher)
{
    PROFILER(InitWorld);
    
    systems.add<RenderSystem>(device);
    systems.add<CameraMovementSystem>();
    systems.configure();
    
    // Setup the position and orientation of the camera.
    glm::mat4 m = glm::rotate(glm::translate(glm::mat4(), -glm::vec3(80.1, 20.1, 140.1)),
                              glm::pi<float>() * 0.20f,
                              glm::vec3(0, 1, 0));
    
    // Create an entity to represent the camera.
    // Render systems will know by the ActiveCamera that this is the camera.
    // They will retrieve the entity's transformation and take it into
    // account when rendering their stuff.
    entityx::Entity camera = entities.create();
    camera.assign<Transform>(m);
    camera.assign<ActiveCamera>();
    
    // Create an entity to represent the terrain.
    TerrainComponent terrainComponent;
    terrainComponent.terrain = std::make_shared<Terrain>(device, dispatcher);
    entityx::Entity terrainEntity = entities.create();
    terrainEntity.assign<TerrainComponent>(terrainComponent);
    terrainEntity.assign<Transform>();
}

void World::update(entityx::TimeDelta dt)
{
    systems.update<CameraMovementSystem>(dt);
    systems.update<RenderSystem>(dt);
}
