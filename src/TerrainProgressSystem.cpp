//
//  TerrainProgressSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#include "TerrainProgressSystem.hpp"
#include "Transform.hpp"
#include "Renderer/UntexturedVertex.hpp"
#include <glm/gtx/transform.hpp>

static RenderableStaticWireframeMesh
createWireframeBox(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
{
    // Create a mesh for a cube.
    // TODO: Consolidate with createCursorMesh() method in World.cpp
    
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
    
    cubeMesh.hidden = false;
    cubeMesh.vertexCount = unpackedVertices.size();
    cubeMesh.vertexBuffer = vertexBuffer;
    cubeMesh.uniforms = uniformBuffer;
    cubeMesh.shader = shader;
    
    return cubeMesh;
}

TerrainProgressSystem::TerrainProgressSystem(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
 : _graphicsDevice(graphicsDevice)
{}

void TerrainProgressSystem::configure(entityx::EventManager &eventManager)
{
    eventManager.subscribe<TerrainProgressEvent>(*this);
}

void TerrainProgressSystem::update(entityx::EntityManager &es,
                                   entityx::EventManager &events,
                                   entityx::TimeDelta deltaMilliseconds)
{
    while (!_commands.empty()) {
        auto command = _commands.front();
        _commands.pop_front();
        command(es);
    }
}

void TerrainProgressSystem::receive(const TerrainProgressEvent &event)
{
    auto iter = _mapCellToEntity.find(event.cellCoords);
    
    if (iter == _mapCellToEntity.end()) {
        // A corresponding entity does not exist.
        if (event.state != TerrainProgressEvent::Complete) {
            // Create the entity. We queue this to take place in the update()
            // method where we have access to the entity manager.
            glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1), event.boundingBox.center), event.boundingBox.extent*2.f);
            auto mesh = createWireframeBox(_graphicsDevice);
            _commands.emplace_back([&map=_mapCellToEntity, cellCoords=event.cellCoords, mesh=std::move(mesh), transform=std::move(transform)](entityx::EntityManager &es){
                
                Transform transformComponent;
                transformComponent.value = transform;
                
                entityx::Entity testBox = es.create();
                testBox.assign<Transform>(transform);
                testBox.assign<RenderableStaticWireframeMesh>(mesh);
                
                map[cellCoords] = testBox;
            });
        }
    } else {
        // A corresponding entity does exist, and we can use that.
        if (event.state == TerrainProgressEvent::Complete) {
            // Remove the entity as we no longer need the wireframe box.
            entityx::Entity &entity = iter->second;
            entity.destroy();
            _mapCellToEntity.erase(iter);
        }
    }
}
