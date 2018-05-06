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
#include "TerrainCursorSystem.hpp"
#include "TerrainProgressSystem.hpp"
#include "TerrainComponent.hpp"
#include "TerrainCursor.hpp"
#include "Profiler.hpp"
#include "WireframeCube.hpp"

#include <glm/gtc/matrix_transform.hpp>

World::World(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
             const std::shared_ptr<TaskDispatcher> &dispatcherHighPriority,
             const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData,
             const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher)
{
    PROFILER(InitWorld);
    
    auto wireframeCube = std::make_shared<WireframeCube>(graphicsDevice);
    
    systems.add<RenderSystem>(graphicsDevice, wireframeCube);
    systems.add<CameraMovementSystem>();
    systems.add<TerrainCursorSystem>(dispatcherHighPriority,
                                     mainThreadDispatcher);
    systems.add<TerrainProgressSystem>(wireframeCube);
    systems.configure();
    
    // Setup the position and orientation of the camera.
    const glm::vec3 cameraPosition = glm::vec3(80.1, 20.1, 140.1);
    glm::mat4 m = glm::rotate(glm::translate(glm::mat4(), -cameraPosition),
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
    terrainComponent.terrain = std::make_shared<Terrain>(graphicsDevice,
                                                         dispatcherHighPriority,
                                                         dispatcherVoxelData,
                                                         mainThreadDispatcher,
                                                         events,
                                                         cameraPosition);
    entityx::Entity terrainEntity = entities.create();
    {
        terrainEntity.assign<TerrainComponent>(terrainComponent);
        terrainEntity.assign<Transform>();
    }
    
    entityx::Entity terrainCursor = entities.create();
    {
        terrainCursor.assign<Transform>();
        terrainCursor.assign<TerrainCursor>();
        terrainCursor.component<TerrainCursor>()->terrainEntity = terrainEntity;
        terrainCursor.assign<WireframeCube::Renderable>(wireframeCube->createMesh());
    }
}

void World::update(entityx::TimeDelta dt)
{
    systems.update<CameraMovementSystem>(dt);
    systems.update<TerrainCursorSystem>(dt);
    systems.update<TerrainProgressSystem>(dt);
    systems.update<RenderSystem>(dt);
}
