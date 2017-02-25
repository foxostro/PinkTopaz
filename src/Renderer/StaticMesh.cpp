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

static const uint32_t GEO_MAGIC = 'moeg';
static const uint32_t GEO_VERSION = 0;

namespace PinkTopaz::Renderer {
    
    StaticMesh::~StaticMesh()
    {
        // Do nothing
    }
    
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
    
    const StaticMesh::Header *StaticMesh::getHeader() const
    {
        StaticMesh::Header *header = (StaticMesh::Header *)_bytes.data();
        return header;
    }

} // namespace PinkTopaz::Renderer
