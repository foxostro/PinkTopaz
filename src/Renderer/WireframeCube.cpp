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
#include <deque>
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
    
    // How many UniformsPerInstance can we pack into a single uniform buffer?
    {
        size_t maxUniformBufferSize = _graphicsDevice->getMaxBufferSize(UniformBuffer);
        _maxNumberOfUniformsPerInstancePerUniformBuffer = maxUniformBufferSize / sizeof(UniformsPerInstance);
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
    // Build per-instance uniform data.
    std::deque<UniformsPerInstance> uniformsPerInstance;
    
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
    
    if (uniformsPerInstance.size() == 0) {
        return; // nothing to do
    }
    
    // The uniforms shared between all instances.
    WireframeCube::Uniforms uniforms = {
        projectionMatrix
    };
    _uniformBuffer->replace(sizeof(uniforms), &uniforms);
    
    // Setup to draw the batches.
    encoder->setTriangleFillMode(Lines);
    encoder->setShader(_shader);
    encoder->setVertexBuffer(_vertexBuffer, 0);
    encoder->setVertexBuffer(_uniformBuffer, 1);
    
    // We'll need to break the per-instance data up into multiple buffers as it
    // may not fit into a single buffer. (This really only happens when using
    // the OpenGL renderer since we can't use Shader Storage Buffer Objects.)
    
    std::vector<std::shared_ptr<Buffer>> pool = _uniformsPerInstanceBufferPool;
    
    while (uniformsPerInstance.size() > 0) {
        std::vector<UniformsPerInstance> uniformsPerInstanceBatch;
        uniformsPerInstanceBatch.reserve(_maxNumberOfUniformsPerInstancePerUniformBuffer);
        for (size_t i = 0, n = std::min(_maxNumberOfUniformsPerInstancePerUniformBuffer, uniformsPerInstance.size()); i < n; ++i) {
            UniformsPerInstance perInstanceData = uniformsPerInstance.front();
            uniformsPerInstance.pop_front();
            uniformsPerInstanceBatch.push_back(perInstanceData);
        }
        
        // Grab and update a uniform buffer from the pool, or create a new one.
        std::shared_ptr<Buffer> buffer;
        if (pool.size() == 0) {
            buffer = _graphicsDevice->makeBuffer(uniformsPerInstanceBatch.size() * sizeof(UniformsPerInstance),
                                                 uniformsPerInstanceBatch.data(),
                                                 DynamicDraw,
                                                 UniformBuffer);
            _uniformsPerInstanceBufferPool.push_back(buffer);
        } else {
            buffer = pool.back();
            pool.pop_back();
            buffer->replace(uniformsPerInstanceBatch.size() * sizeof(UniformsPerInstance),
                            uniformsPerInstanceBatch.data());
        }
        
        // Bind the per-instance uniform buffer and draw.
        encoder->setVertexBuffer(buffer, 2);
        encoder->drawIndexedPrimitives(TriangleStrip,
                                       _indexCount,
                                       _indexBuffer,
                                       uniformsPerInstanceBatch.size());
    }
    
    // Reset
    encoder->setTriangleFillMode(Fill);
}
