//
//  WireframeCube.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#include "WireframeCube.hpp"
#include <array>
#include <vector>

WireframeCube::WireframeCube(std::shared_ptr<GraphicsDevice> graphicsDevice)
 : _graphicsDevice(graphicsDevice)
{}

WireframeCube::Renderable WireframeCube::createMesh()
{
    static const float L = 0.5f;
    
    const std::array<size_t, 14> indices = {{
        3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0
    }};
    
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
    
    // TODO: Pass the index buffer to the GPU and use indexed drawing instead.
    const std::array<Vertex, 14> unpackedVertices = {{
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
    
    const size_t bufferSize = unpackedVertices.size() * sizeof(Vertex);
    std::vector<uint8_t> vertexBufferData(bufferSize);
    memcpy(&vertexBufferData[0], &unpackedVertices[0], bufferSize);
    
    auto vertexBuffer = _graphicsDevice->makeBuffer(vertexBufferData,
                                                    StaticDraw,
                                                    ArrayBuffer);
    vertexBuffer->addDebugMarker("Wireframe Sube Vertices", 0, bufferSize);
    
    VertexFormat vertexFormat;
    vertexFormat.attributes.emplace_back(AttributeFormat{
        /* .size = */ 4,
        /* .type = */ AttributeTypeFloat,
        /* .normalized = */ false,
        /* .stride = */ sizeof(Vertex),
        /* .offset = */ offsetof(Vertex, position)
    });
    
    auto shader = _graphicsDevice->makeShader(vertexFormat,
                                              "wireframe_cube_vert",
                                              "wireframe_cube_frag",
                                              /* blending = */ false);
    
    Uniforms uniforms;
    auto uniformBuffer = _graphicsDevice->makeBuffer(sizeof(uniforms),
                                                     &uniforms,
                                                     DynamicDraw,
                                                     UniformBuffer);
    uniformBuffer->addDebugMarker("Wireframe Cube Uniforms", 0, sizeof(uniforms));
    
    Renderable cubeMesh;
    
    cubeMesh.hidden = false;
    cubeMesh.vertexCount = unpackedVertices.size();
    cubeMesh.vertexBuffer = vertexBuffer;
    cubeMesh.uniforms = uniformBuffer;
    cubeMesh.shader = shader;
    cubeMesh.color = glm::vec4(1.0f);
    
    return cubeMesh;
}
