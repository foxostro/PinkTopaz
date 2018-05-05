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
#include "Renderer/UntexturedVertex.hpp"

#include <glm/gtc/matrix_transform.hpp>

World::World(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
             const std::shared_ptr<TaskDispatcher> &dispatcherHighPriority,
             const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData,
             const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher)
{
    PROFILER(InitWorld);
    
    systems.add<RenderSystem>(graphicsDevice);
    systems.add<CameraMovementSystem>();
    systems.add<TerrainCursorSystem>(dispatcherHighPriority,
                                     mainThreadDispatcher);
    systems.add<TerrainProgressSystem>(graphicsDevice);
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
    terrainEntity.assign<TerrainComponent>(terrainComponent);
    terrainEntity.assign<Transform>();
    
    entityx::Entity terrainCursor = entities.create();
    terrainCursor.assign<Transform>();
    terrainCursor.assign<TerrainCursor>();
    terrainCursor.component<TerrainCursor>()->terrainEntity = terrainEntity;
    terrainCursor.assign<RenderableStaticWireframeMesh>(createCursorMesh(graphicsDevice));
    
    entityx::Entity testBox = entities.create();
    testBox.assign<Transform>();
    testBox.component<Transform>()->value = glm::translate(glm::mat4(1), glm::vec3(80.1, 20.1, 140.1));
    auto testBoxMesh = createCursorMesh(graphicsDevice);
    testBoxMesh.hidden = false;
    testBox.assign<RenderableStaticWireframeMesh>(testBoxMesh);
}

void World::update(entityx::TimeDelta dt)
{
    systems.update<CameraMovementSystem>(dt);
    systems.update<TerrainCursorSystem>(dt);
    systems.update<TerrainProgressSystem>(dt);
    systems.update<RenderSystem>(dt);
}

RenderableStaticWireframeMesh World::createCursorMesh(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
{
    // Create a mesh for a cube.
    
    static const float L = 0.5;
    
    const std::array<size_t, 14> indices = {{
        3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0
    }};
    
    const std::array<glm::vec4, 8> vertices = {{
        glm::vec4(-L,  L, -L, 1),
        glm::vec4( L,  L, -L, 1),
        glm::vec4(-L,  L,  L, 1),
        glm::vec4( L,  L,  L, 1),
        glm::vec4(-L, -L, -L, 1),
        glm::vec4( L, -L, -L, 1),
        glm::vec4( L, -L,  L, 1),
        glm::vec4(-L, -L,  L, 1)
    }};
    
    // TODO: Pass the index buffer to the GPU and use indexed drawing instead.
    const std::array<UntexturedVertex, 14> unpackedVertices = {{
        {vertices[indices[0]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[1]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[2]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[3]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[4]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[5]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[6]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[7]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[8]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[9]],  glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[10]], glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[11]], glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[12]], glm::vec4(1.f, 1.f, 1.f, 1.f)},
        {vertices[indices[13]], glm::vec4(1.f, 1.f, 1.f, 1.f)}
    }};
    
    const size_t bufferSize = unpackedVertices.size() * sizeof(UntexturedVertex);
    std::vector<uint8_t> vertexBufferData(bufferSize);
    memcpy(&vertexBufferData[0], &unpackedVertices[0], bufferSize);
    
    auto vertexBuffer = graphicsDevice->makeBuffer(vertexBufferData,
                                                    StaticDraw,
                                                    ArrayBuffer);
    
    VertexFormat vertexFormat;
    {
        AttributeFormat attr = {
            4,
            AttributeTypeFloat,
            false,
            sizeof(UntexturedVertex),
            offsetof(UntexturedVertex, position)
        };
        vertexFormat.attributes.emplace_back(attr);
        
        attr = {
            4,
            AttributeTypeFloat,
            false,
            sizeof(UntexturedVertex),
            offsetof(UntexturedVertex, color)
        };
        vertexFormat.attributes.emplace_back(attr);
    }
    
    auto shader = graphicsDevice->makeShader(vertexFormat,
                                              "untextured_vert",
                                              "untextured_frag",
                                              /* blending = */ false);
    
    UntexturedUniforms uniforms;
    auto uniformBuffer = graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
    uniformBuffer->addDebugMarker("Cube Uniforms", 0, sizeof(uniforms));
    
    RenderableStaticWireframeMesh cubeMesh;
    
    cubeMesh.hidden = true;
    cubeMesh.vertexCount = unpackedVertices.size();
    cubeMesh.vertexBuffer = vertexBuffer;
    cubeMesh.uniforms = uniformBuffer;
    cubeMesh.shader = shader;
    
    return cubeMesh;
}
