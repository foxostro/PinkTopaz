//
//  StaticMeshLoader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#ifndef StaticMeshLoader_hpp
#define StaticMeshLoader_hpp

#include "Renderer/StaticMesh.hpp"

namespace PinkTopaz::Renderer {
    
    class StaticMeshLoader
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
        
        StaticMeshLoader(const char *filePath);
        ~StaticMeshLoader() = default;
        
        const Header& getHeader() const;
        std::vector<TerrainVertex> getVertices() const;
        std::vector<uint8_t> getBufferData() const;
        
        inline size_t getVertexCount() const
        {
            size_t count = getHeader().numVerts;
            return count;
        }
        
        StaticMesh getStaticMesh() const;
        
    private:
        const uint32_t GEO_MAGIC, GEO_VERSION;
        std::vector<uint8_t> _bytes;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMesh_hpp */
