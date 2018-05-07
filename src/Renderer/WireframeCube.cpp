//
//  WireframeCube.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#include "WireframeCube.hpp"
#include "Transform.hpp"
#include <array>
#include <vector>

WireframeCube::WireframeCube(std::shared_ptr<GraphicsDevice> graphicsDevice)
 : _graphicsDevice(graphicsDevice)
{
    // Create the index buffer.
    {
        const std::array<uint32_t, 14> indices = {{
            3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0
        }};
        
        _indexCount = indices.size();
        const size_t bufferSize = indices.size() * sizeof(uint32_t);
        _indexBuffer = _graphicsDevice->makeBuffer(bufferSize,
                                                   &indices[0],
                                                   StaticDraw,
                                                   IndexBuffer);
        _indexBuffer->addDebugMarker("Wireframe Cube Indices", 0, bufferSize);
    }
    
    // Create the vertex buffer.
    {
        constexpr float L = 0.5f;
        const std::array<Vertex, 8> vertices = {{
            Vertex(glm::vec4(-L,  L, -L, 1)),
            Vertex(glm::vec4( L,  L, -L, 1)),
            Vertex(glm::vec4(-L,  L,  L, 1)),
            Vertex(glm::vec4( L,  L,  L, 1)),
            Vertex(glm::vec4(-L, -L, -L, 1)),
            Vertex(glm::vec4( L, -L, -L, 1)),
            Vertex(glm::vec4( L, -L,  L, 1)),
            Vertex(glm::vec4(-L, -L,  L, 1))
        }};
        
        const size_t bufferSize = vertices.size() * sizeof(Vertex);
        _vertexBuffer = _graphicsDevice->makeBuffer(bufferSize,
                                                    &vertices[0],
                                                    StaticDraw,
                                                    ArrayBuffer);
        _vertexBuffer->addDebugMarker("Wireframe Cube Vertices", 0, bufferSize);
    }
    
    // Create the buffer for uniforms shared between instances.
    {
        Uniforms uniforms;
        _uniformBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
        _uniformBuffer->addDebugMarker("Wireframe Cube Shared Uniforms", 0, sizeof(uniforms));
    }
    
    // Create the buffer for per-instance uniforms.
    {
        UniformsPerInstance uniforms;
        _uniformsPerInstanceBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                                 &uniforms,
                                                                 DynamicDraw,
                                                                 UniformBuffer);
    }
    
    // Create the shader.
    {
        VertexFormat vertexFormat;
        vertexFormat.attributes.emplace_back(AttributeFormat{
            /* .size = */ 4,
            /* .type = */ AttributeTypeFloat,
            /* .normalized = */ false,
            /* .stride = */ sizeof(Vertex),
            /* .offset = */ offsetof(Vertex, position)
        });
        
        _shader = _graphicsDevice->makeShader(vertexFormat,
                                              "wireframe_cube_vert",
                                              "wireframe_cube_frag",
                                              /* blending = */ false);
    }
}

WireframeCube::Renderable WireframeCube::createMesh()
{
    const glm::vec4 defaultColor(1.0f);
    Renderable mesh;
    mesh.hidden = false;
    mesh.modelView = glm::mat4(1.f); // identity
    mesh.color = defaultColor;
    return mesh;
}

void WireframeCube::draw(entityx::EntityManager &es,
                         const std::shared_ptr<CommandEncoder> &encoder,
                         const glm::mat4 &projectionMatrix,
                         const glm::mat4 &cameraTransform)
{
    // Build per-instance uniform data and then submit to the GPU.
    std::vector<UniformsPerInstance> uniformsPerInstance;
    
    es.each<Renderable, Transform>([&](entityx::Entity entity,
                                       Renderable &mesh,
                                       Transform &transform) {
        if (!mesh.hidden) {
            UniformsPerInstance uniforms;
            uniforms.modelView = cameraTransform * transform.value;
            uniforms.color = mesh.color;
            uniformsPerInstance.push_back(uniforms);
        }
    });
    
    const size_t instanceCount = uniformsPerInstance.size();
    
    if (instanceCount == 0) {
        return; // nothing to do
    }
    
    _uniformsPerInstanceBuffer->replace(instanceCount * sizeof(UniformsPerInstance),
                                        uniformsPerInstance.data());
    
    // The uniforms shared between all instances.
    WireframeCube::Uniforms uniforms = {
        projectionMatrix
    };
    _uniformBuffer->replace(sizeof(uniforms), &uniforms);
    
    encoder->setTriangleFillMode(Lines);
    encoder->setShader(_shader);
    encoder->setVertexBuffer(_vertexBuffer, 0);
    encoder->setVertexBuffer(_uniformBuffer, 1);
    encoder->setVertexBuffer(_uniformsPerInstanceBuffer, 2);
    encoder->drawIndexedPrimitives(TriangleStrip, _indexCount,
                                   _indexBuffer, instanceCount);
    encoder->setTriangleFillMode(Fill); // restore
}
