//
//  StaticMesh.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#ifndef StaticMesh_hpp
#define StaticMesh_hpp

#include <vector>
#include "Renderer/VertexFormat.hpp"

namespace PinkTopaz::Renderer {
    
    class StaticMesh
    {
    public:
        struct Vertex
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
            Vertex vertices[0];
        };
        
        StaticMesh(const char *filePath);
        ~StaticMesh() = default;
        
        const Header *getHeader() const;
        VertexFormat getVertexFormat() const;
        std::vector<uint8_t> getBufferData() const;
        
        inline size_t getVertexCount() const
        {
            size_t count = getHeader()->numVerts;
            return count;
        }
        
    private:
        std::vector<uint8_t> _bytes;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMesh_hpp */
