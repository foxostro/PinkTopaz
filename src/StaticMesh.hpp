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
#include "opengl.hpp"

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
    
    // The StaticMeshVAO instance must be used only on the OpenGL thread.
    class StaticMeshVAO
    {
    public:
        StaticMeshVAO(const StaticMesh &mesh);
        ~StaticMeshVAO();

        GLuint getVAO() const
        {
            return _vao;
        }
        
        GLsizei getNumVerts() const
        {
            return _numVerts;
        }

    private:
        GLuint _vao;
        GLsizei _numVerts;
    };
    
} // namespace PinkTopaz

#endif /* StaticMesh_hpp */
