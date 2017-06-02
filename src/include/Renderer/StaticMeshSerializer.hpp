//
//  StaticMeshSerializer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#ifndef StaticMeshSerializer_hpp
#define StaticMeshSerializer_hpp

#include "Renderer/StaticMesh.hpp"
#include <boost/filesystem.hpp>

class StaticMeshSerializer
{
public:
    struct FileVertex
    {
        float position[3];
        unsigned char color[4];
        float texCoord[3];
    };
    
    struct Header
    {
        uint32_t magic;
        uint32_t version;
        uint32_t w, h, d;
        int32_t numVerts;
        uint32_t len;
        FileVertex vertices[0];
    };
    
    StaticMeshSerializer();
    ~StaticMeshSerializer() = default;
    
    StaticMesh load(const std::vector<uint8_t> &bytes);
    std::vector<uint8_t> save(const StaticMesh &mesh);
    
private:
    const uint32_t GEO_MAGIC, GEO_VERSION;
    
    void convert(const FileVertex &input, TerrainVertex &output) const;
    void convert(const TerrainVertex &input, FileVertex &output) const;
};

#endif /* StaticMeshSerializer_hpp */
