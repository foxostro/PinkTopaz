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
    // TODO: Render an instanced wireframe box for each chunk. If we do this then each box only consumes one vec4 for the position and one vec4 for the color. Also, we can use this as an opportunity to stop creating and destroying uniforms buffers every time a cube component is created and destroyed. (Admittedly, we could do this without also doing instancing if we pooled and reused the uniforms buffers.)
    
    const glm::vec4 defaultColor(1.0f);
    
    Uniforms uniforms;
    uniforms.color = defaultColor;
    auto uniformBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
    uniformBuffer->addDebugMarker("Wireframe Cube Uniforms", 0, sizeof(uniforms));
    
    Renderable mesh;
    
    mesh.hidden = false;
    mesh.uniforms = uniformBuffer;
    mesh.color = glm::vec4(1.f);
    
    return mesh;
}

void WireframeCube::draw(entityx::EntityManager &es,
                         const std::shared_ptr<CommandEncoder> &encoder,
                         const glm::mat4 &projectionMatrix,
                         const glm::mat4 &cameraTransform)
{
    es.each<Renderable, Transform>([&](entityx::Entity entity,
                                       Renderable &mesh,
                                       Transform &transform) {
        WireframeCube::Uniforms uniforms = {
            cameraTransform * transform.value,
            projectionMatrix,
            mesh.color
        };
        mesh.uniforms->replace(sizeof(uniforms), &uniforms);
    });
    
    encoder->setTriangleFillMode(Lines);
    encoder->setShader(_shader);
    encoder->setVertexBuffer(_vertexBuffer, 0);
    
    es.each<Renderable>([&](entityx::Entity entity, Renderable &mesh){
        if (!mesh.hidden) {
            encoder->setVertexBuffer(mesh.uniforms, 1);
            encoder->drawIndexedPrimitives(/* primitiveType= */ TriangleStrip,
                                           /* indexCount= */ _indexCount,
                                           /* indexBuffer= */ _indexBuffer,
                                           /* instanceCount= */ 1);
        }
    });
    
    encoder->setTriangleFillMode(Fill); // restore
}
