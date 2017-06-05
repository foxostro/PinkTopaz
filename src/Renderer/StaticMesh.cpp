//
//  StaticMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include "Renderer/StaticMesh.hpp"

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
    
    AttributeFormat attr = {
        4,
        AttributeTypeFloat,
        false,
        sizeof(TerrainVertex),
        offsetof(TerrainVertex, position)
    };
    _vertexFormat.attributes.emplace_back(attr);
    
    attr = {
        4,
        AttributeTypeFloat,
        false,
        sizeof(TerrainVertex),
        offsetof(TerrainVertex, color)
    };
    _vertexFormat.attributes.emplace_back(attr);
    
    attr = {
        3,
        AttributeTypeFloat,
        false,
        sizeof(TerrainVertex),
        offsetof(TerrainVertex, texCoord)
    };
    _vertexFormat.attributes.emplace_back(attr);
}

bool StaticMesh::operator==(const StaticMesh &other) const
{
    if (&other == this) {
        return true;
    }
    
    return _vertices == other._vertices;
}
