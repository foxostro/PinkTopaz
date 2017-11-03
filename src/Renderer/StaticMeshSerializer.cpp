//
//  StaticMeshSerializer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#include "SDL.h"
#include "FileUtilities.hpp"
#include "Exception.hpp"
#include "Renderer/StaticMeshSerializer.hpp"

StaticMeshSerializer::StaticMeshSerializer()
 : GEO_MAGIC('moeg'), GEO_VERSION(0)
{}

StaticMesh StaticMeshSerializer::load(const std::vector<uint8_t> &bytes)
{
    const Header &header = *((Header *)bytes.data());
    
    if (header.magic != GEO_MAGIC) {
        throw Exception("Unexpected magic number in geometry data file: found %d but expected %d", header.magic, GEO_MAGIC);
    }
    
    if (header.version != GEO_VERSION) {
        throw Exception("Unexpected version number in geometry data file: found %d but expected %d", header.version, GEO_VERSION);
    }
    
    if (header.len != (header.numVerts * sizeof(FileVertex))) {
        throw Exception("Unexpected number of bytes used in geometry data file.");
    }
    
    std::vector<TerrainVertex> vertices(header.numVerts);
    
    for (size_t i = 0, n = header.numVerts; i < n; ++i)
    {
        TerrainVertex &gpuVertex = vertices[i];
        const FileVertex &fileVertex = header.vertices[i];
        convert(fileVertex, gpuVertex);
    }
    
    return StaticMesh(vertices);
}

std::vector<uint8_t> StaticMeshSerializer::save(const StaticMesh &mesh)
{
    const std::vector<TerrainVertex> &vertices = mesh.getVertices();
    
    std::vector<uint8_t> bytes;
    
    bytes.resize(vertices.size() * sizeof(FileVertex) + sizeof(Header));
    
    Header &header = *((Header *)bytes.data());
    header.magic = GEO_MAGIC;
    header.version = GEO_VERSION;
    header.w = 256; // unused field
    header.h = 256; // unused field
    header.d = 256; // unused field
    header.numVerts = (int32_t)vertices.size();
    header.len = (uint32_t)(vertices.size() * sizeof(FileVertex));
    
    for (size_t i = 0, n = header.numVerts; i < n; ++i)
    {
        FileVertex &fileVertex = header.vertices[i];
        const TerrainVertex &gpuVertex = vertices[i];
        convert(gpuVertex, fileVertex);
    }
    
    return bytes;
}

void StaticMeshSerializer::convert(const FileVertex &input,
                                   TerrainVertex &output) const
{
    output.position[0] = input.position[0];
    output.position[1] = input.position[1];
    output.position[2] = input.position[2];
    output.position[3] = 1.0;
    
    output.color[0] = input.color[0] / 255.0f;
    output.color[1] = input.color[1] / 255.0f;
    output.color[2] = input.color[2] / 255.0f;
    output.color[3] = input.color[3] / 255.0f;
    
    output.texCoord[0] = input.texCoord[0];
    output.texCoord[1] = input.texCoord[1];
    output.texCoord[2] = input.texCoord[2];
}

void StaticMeshSerializer::convert(const TerrainVertex &input,
                                   FileVertex &output) const
{
    output.position[0] = input.position[0];
    output.position[1] = input.position[1];
    output.position[2] = input.position[2];
    // input.position[3] is not preserved
    
    output.color[0] = input.color[0] * 255.0f;
    output.color[1] = input.color[1] * 255.0f;
    output.color[2] = input.color[2] * 255.0f;
    output.color[3] = input.color[3] * 255.0f;
    
    output.texCoord[0] = input.texCoord[0];
    output.texCoord[1] = input.texCoord[1];
    output.texCoord[2] = input.texCoord[2];
}
