//
//  StaticMeshVaoOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include "Exception.hpp"
#include "Renderer/OpenGL/StaticMeshVaoOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Renderer/StaticMesh.hpp"
#include "SDL.h"
#include <cassert>

namespace PinkTopaz::Renderer::OpenGL {
    
    StaticMeshVaoOpenGL::StaticMeshVaoOpenGL(CommandQueue &queue, const std::shared_ptr<StaticMesh> &mesh)
     : _vao(0),
       _numVerts(0),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            const GLsizei stride = sizeof(StaticMesh::Vertex);
            
            const StaticMesh::Header &header = *(mesh->getHeader());
            _numVerts = header.numVerts;
            GLsizeiptr bufferSize = header.numVerts * stride;
            const GLvoid *vertsBuffer = (const GLvoid *)header.vertices;
            
            const GLvoid *offsetVertex   = (const GLvoid *)offsetof(StaticMesh::Vertex, position);
            const GLvoid *offsetTexCoord = (const GLvoid *)offsetof(StaticMesh::Vertex, texCoord);
            const GLvoid *offsetColor    = (const GLvoid *)offsetof(StaticMesh::Vertex, color);
            
            GLuint vao = 0;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            GLuint vbo = 0;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, vertsBuffer, GL_STATIC_DRAW);
            
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT,         GL_FALSE, stride, offsetVertex);
            glVertexAttribPointer(1, 3, GL_FLOAT,         GL_FALSE, stride, offsetTexCoord);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, stride, offsetColor);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glBindVertexArray(0);
            glDeleteBuffers(1, &vbo); // Release our reference after the VAO is initialized.
            
            this->_vao = vao;
            
            CHECK_GL_ERROR();
        });
    }
    
    StaticMeshVaoOpenGL::~StaticMeshVaoOpenGL()
    {
        GLuint handle = _vao;
        _commandQueue.enqueue([handle]{
            glDeleteVertexArrays(1, &handle);
        });
    }
    
    size_t StaticMeshVaoOpenGL::getNumVerts() const
    {
        return _numVerts;
    }

} // namespace PinkTopaz::Renderer
