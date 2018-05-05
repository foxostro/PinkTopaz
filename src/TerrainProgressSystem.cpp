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
#include "Exception.hpp"
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
    
    const std::array<UntexturedVertex, 8> vertices = {{
        UntexturedVertex(glm::vec4(-L,  L, -L, 1)),
        UntexturedVertex(glm::vec4( L,  L, -L, 1)),
        UntexturedVertex(glm::vec4(-L,  L,  L, 1)),
        UntexturedVertex(glm::vec4( L,  L,  L, 1)),
        UntexturedVertex(glm::vec4(-L, -L, -L, 1)),
        UntexturedVertex(glm::vec4( L, -L, -L, 1)),
        UntexturedVertex(glm::vec4( L, -L,  L, 1)),
        UntexturedVertex(glm::vec4(-L, -L,  L, 1))
    }};
    
    // TODO: Pass the index buffer to the GPU and use indexed drawing instead.
    const std::array<UntexturedVertex, 14> unpackedVertices = {{
        vertices[indices[0]],
        vertices[indices[1]],
        vertices[indices[2]],
        vertices[indices[3]],
        vertices[indices[4]],
        vertices[indices[5]],
        vertices[indices[6]],
        vertices[indices[7]],
        vertices[indices[8]],
        vertices[indices[9]],
        vertices[indices[10]],
        vertices[indices[11]],
        vertices[indices[12]],
        vertices[indices[13]]
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
    cubeMesh.color = glm::vec4(1.0f);
    
    return cubeMesh;
}

TerrainProgressSystem::TerrainProgressSystem(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
 : _graphicsDevice(graphicsDevice)
{
    _mapStateToColor[TerrainProgressEvent::Queued] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::WaitingOnVoxels] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::ExtractingSurface] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::Complete] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
}

void TerrainProgressSystem::configure(entityx::EventManager &eventManager)
{
    eventManager.subscribe<TerrainProgressEvent>(*this);
}

void TerrainProgressSystem::update(entityx::EntityManager &es,
                                   entityx::EventManager &events,
                                   entityx::TimeDelta deltaMilliseconds)
{
    while (!_pendingEvents.empty()) {
        auto event = _pendingEvents.front();
        handleEvent(es, event);
        _pendingEvents.pop_front();
    }
}

void TerrainProgressSystem::receive(const TerrainProgressEvent &event)
{
    // Queue this to take place in update() where we have access to the
    // entity manager.
    _pendingEvents.push_back(event);
}

void TerrainProgressSystem::handleEvent(entityx::EntityManager &es,
                                        const TerrainProgressEvent &event)
{
    auto iter = _mapCellToEntity.find(event.cellCoords);
    
    if (iter == _mapCellToEntity.end()) {
        // A corresponding entity does not exist. Create it now if the event
        // puts the chunk into a state where we want to show progress.
        if (event.state != TerrainProgressEvent::Complete) {
            entityx::Entity entity = es.create();
            
            Transform transform;
            transform.value = glm::scale(glm::translate(glm::mat4(1), event.boundingBox.center), event.boundingBox.extent*2.f);
            entity.assign<Transform>(transform);
            
            // TODO: Render an instanced wireframe box for each chunk. If we do this then each box only consumes one vec4 for the position and one vec4 for the color.
            auto mesh = createWireframeBox(_graphicsDevice);
            entity.assign<RenderableStaticWireframeMesh>(mesh);
            
            _mapCellToEntity[event.cellCoords] = std::move(entity);
        }
    } else {
        entityx::Entity entity = iter->second;
        
        entity.component<RenderableStaticWireframeMesh>()->color = _mapStateToColor[event.state];
        
        // If the chunk is "complete" then remove the entity.
        // We no longer need the wireframe box.
        if (event.state == TerrainProgressEvent::Complete) {
            entity.destroy();
            
            auto iter = _mapCellToEntity.find(event.cellCoords);
            
            if (iter != _mapCellToEntity.end()) {
                _mapCellToEntity.erase(iter);
            } else {
                throw Exception("How did we get an invalid iterator here?");
            }
        }
    }
}
