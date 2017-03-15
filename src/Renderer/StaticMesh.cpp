//
//  StaticMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include "Renderer/StaticMesh.hpp"

namespace PinkTopaz::Renderer {
    
    StaticMesh::StaticMesh()
    {
        initVertexFormat();
    }
    
    StaticMesh::StaticMesh(const std::vector<TerrainVertex> &vertices)
     : _vertices(vertices)
    {
        initVertexFormat();
    }
    
    std::vector<uint8_t> StaticMesh::getBufferData() const
    {
        const std::vector<TerrainVertex> &vertices = getVertices();
        const size_t bufferSize = vertices.size() * sizeof(TerrainVertex);
        std::vector<uint8_t> bufferData(bufferSize);
        memcpy(&bufferData[0], &vertices[0], bufferSize);
        return bufferData;
    }
    
    void StaticMesh::initVertexFormat()
    {
        _vertexFormat.attributes.clear();
        
        _vertexFormat.attributes.emplace_back((AttributeFormat){
            .size = 4,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(TerrainVertex),
            .offset = offsetof(TerrainVertex, position)
        });
        
        _vertexFormat.attributes.emplace_back((AttributeFormat){
            .size = 4,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(TerrainVertex),
            .offset = offsetof(TerrainVertex, color)
        });
        
        _vertexFormat.attributes.emplace_back((AttributeFormat){
            .size = 3,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(TerrainVertex),
            .offset = offsetof(TerrainVertex, texCoord)
        });
    }

} // namespace PinkTopaz::Renderer
