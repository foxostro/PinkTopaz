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
        ~StaticMesh();
        
        const Header *getHeader() const;
        
    private:
        std::vector<uint8_t> _bytes;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMesh_hpp */
