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
#include <OpenGL/gl3.h>

namespace PinkTopaz {
    
    class StaticMesh
    {
    public:
        struct Vertex
        {
            GLfloat position[3];
            GLubyte color[4];
            GLfloat texCoord[3];
        };
        
        struct Header
        {
            uint32_t magic;
            uint32_t version;
            uint32_t w, h, d;
            GLsizei numVerts;
            uint32_t len;
            Vertex vertices[0];
        };
        
        StaticMesh(const char *filePath);
        ~StaticMesh();
        
        const Header *getHeader() const;
        
    private:
        std::vector<uint8_t> _bytes;
    };
    
} // namespace PinkTopaz

#endif /* StaticMesh_hpp */
