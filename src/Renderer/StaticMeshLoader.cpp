//
//  StaticMeshLoader.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#include "SDL.h"
#include "FileUtilities.hpp"
#include "Exception.hpp"
#include "Renderer/StaticMeshLoader.hpp"

namespace PinkTopaz::Renderer {
    
    StaticMeshLoader::StaticMeshLoader(const char *filePath)
     : GEO_MAGIC('moeg'), GEO_VERSION(0)
    {
        _bytes = binaryFileContents(filePath);
        
        const Header &header = getHeader();
        
        if (header.magic != GEO_MAGIC) {
            throw Exception("Unexpected magic number in geometry data file: found %d but expected %d", header.magic, GEO_MAGIC);
        }
        
        if (header.version != GEO_VERSION) {
            throw Exception("Unexpected version number in geometry data file: found %d but expected %d", header.version, GEO_VERSION);
        }
        
        if (header.len != (header.numVerts * sizeof(FileVertex))) {
            throw Exception("Unexpected number of bytes used in geometry data file.");
        }
    }
    
    const StaticMeshLoader::Header& StaticMeshLoader::getHeader() const
    {
        return *((Header *)_bytes.data());
    }
    
    std::vector<TerrainVertex> StaticMeshLoader::getVertices() const
    {
        const Header &header = getHeader();
        std::vector<TerrainVertex> vertices(header.numVerts);
        
        for (size_t i = 0, n = header.numVerts; i < n; ++i)
        {
            TerrainVertex &gpuVertex = vertices[i];
            const FileVertex &fileVertex = header.vertices[i];
            
            gpuVertex.position[0] = fileVertex.position[0];
            gpuVertex.position[1] = fileVertex.position[1];
            gpuVertex.position[2] = fileVertex.position[2];
            gpuVertex.position[3] = 1.0;
            
            gpuVertex.color[0] = fileVertex.color[0] / 255.0f;
            gpuVertex.color[1] = fileVertex.color[1] / 255.0f;
            gpuVertex.color[2] = fileVertex.color[2] / 255.0f;
            gpuVertex.color[3] = fileVertex.color[3] / 255.0f;
            
            gpuVertex.texCoord[0] = fileVertex.texCoord[0];
            gpuVertex.texCoord[1] = fileVertex.texCoord[1];
            gpuVertex.texCoord[2] = fileVertex.texCoord[2];
        }
        
        return vertices;
    }
    
    StaticMesh StaticMeshLoader::getStaticMesh() const
    {
        return StaticMesh(getVertices());
    }

} // namespace PinkTopaz::Renderer
