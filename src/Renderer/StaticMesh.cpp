//
//  StaticMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include "SDL.h"
#include "FileUtilities.hpp"
#include "Exception.hpp"
#include "Renderer/StaticMesh.hpp"
#include "Renderer/VertexFormat.hpp"

static const uint32_t GEO_MAGIC = 'moeg';
static const uint32_t GEO_VERSION = 0;

namespace PinkTopaz::Renderer {
    
    StaticMesh::StaticMesh(const char *filePath)
    {
        _bytes = binaryFileContents(filePath);
        
        const StaticMesh::Header *header = getHeader();
        
        if (header->magic != GEO_MAGIC) {
            throw Exception("Unexpected magic number in geometry data file: found %d but expected %d", header->magic, GEO_MAGIC);
        }
        
        if (header->version != GEO_VERSION) {
            throw Exception("Unexpected version number in geometry data file: found %d but expected %d", header->version, GEO_VERSION);
        }
        
        if (header->len != (header->numVerts * sizeof(StaticMesh::FileVertex))) {
            throw Exception("Unexpected number of bytes used in geometry data file.");
        }
    }
    
    const StaticMesh::Header* StaticMesh::getHeader() const
    {
        StaticMesh::Header *header = (StaticMesh::Header *)_bytes.data();
        return header;
    }

    VertexFormat StaticMesh::getVertexFormat() const
    {
        VertexFormat format;
        format.attributes.emplace_back((AttributeFormat){
            .size = 3,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(StaticMesh::GPUVertex),
            .offset = offsetof(StaticMesh::GPUVertex, position)
        });
        format.attributes.emplace_back((AttributeFormat){
            .size = 3,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(StaticMesh::GPUVertex),
            .offset = offsetof(StaticMesh::GPUVertex, texCoord)
        });
        format.attributes.emplace_back((AttributeFormat){
            .size = 4,
            .type = AttributeTypeUnsignedByte,
            .normalized = false,
            .stride = sizeof(StaticMesh::GPUVertex),
            .offset = offsetof(StaticMesh::GPUVertex, color)
        });
        return format;
    }
    
    std::vector<StaticMesh::GPUVertex> StaticMesh::getVertices() const
    {
        const StaticMesh::Header &header = *getHeader();
        std::vector<StaticMesh::GPUVertex> vertices(header.numVerts);
        
        for (size_t i = 0, n = header.numVerts; i < n; ++i)
        {
            StaticMesh::GPUVertex &gpuVertex = vertices[i];
            const StaticMesh::FileVertex &fileVertex = header.vertices[i];
            gpuVertex.position = glm::vec4(fileVertex.position[0],
                                           fileVertex.position[1],
                                           fileVertex.position[2],
                                           1.0);
            gpuVertex.texCoord = glm::vec4(fileVertex.texCoord[0],
                                           fileVertex.texCoord[1],
                                           fileVertex.texCoord[2],
                                           0.0);
            gpuVertex.color = glm::tvec4<uint8_t>(fileVertex.color[0],
                                                  fileVertex.color[1],
                                                  fileVertex.color[2],
                                                  fileVertex.color[3]);
        }
        
        return vertices;
    }
    
    std::vector<uint8_t> StaticMesh::getBufferData() const
    {
        std::vector<StaticMesh::GPUVertex> vertices(getVertices());
        
        const StaticMesh::Header &header = *getHeader();
        const size_t bufferSize = header.numVerts * sizeof(StaticMesh::GPUVertex);
        std::vector<uint8_t> bufferData(bufferSize);
        memcpy(&bufferData[0], &vertices[0], bufferSize);
        return bufferData;
    }

} // namespace PinkTopaz::Renderer
