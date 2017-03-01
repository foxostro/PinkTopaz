//
//  StaticMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include <cassert>
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
        
        if (header->len != (header->numVerts * sizeof(StaticMesh::Vertex))) {
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
            .stride = sizeof(StaticMesh::Vertex),
            .offset = offsetof(StaticMesh::Vertex, position)
        });
        format.attributes.emplace_back((AttributeFormat){
            .size = 3,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = sizeof(StaticMesh::Vertex),
            .offset = offsetof(StaticMesh::Vertex, texCoord)
        });
        format.attributes.emplace_back((AttributeFormat){
            .size = 4,
            .type = AttributeTypeUnsignedByte,
            .normalized = false,
            .stride = sizeof(StaticMesh::Vertex),
            .offset = offsetof(StaticMesh::Vertex, color)
        });
        return format;
    }
    
    std::vector<uint8_t> StaticMesh::getBufferData() const
    {
        const StaticMesh::Header &header = *getHeader();
        size_t bufferSize = header.numVerts * sizeof(StaticMesh::Vertex);
        std::vector<uint8_t> bufferData;
        bufferData.resize(bufferSize);
        memcpy(&bufferData[0], header.vertices, bufferSize);
        return bufferData;
    }

} // namespace PinkTopaz::Renderer
