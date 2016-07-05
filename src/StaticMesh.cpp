//
//  StaticMesh.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include <cassert>
#include "FileUtilities.hpp"
#include "StaticMesh.hpp"
#include "Exception.hpp"
#include "glUtilities.hpp"
#include "SDL.h"

static const uint32_t GEO_MAGIC = 'moeg';
static const uint32_t GEO_VERSION = 0;

namespace PinkTopaz {
    
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
    
    StaticMeshVAO::StaticMeshVAO(const StaticMesh &mesh)
    {
        const GLsizei stride = sizeof(StaticMesh::Vertex);
        
        const StaticMesh::Header &header = *mesh.getHeader();
        _numVerts = header.numVerts;
        GLsizeiptr bufferSize = header.numVerts * stride;
        const GLvoid *vertsBuffer = (const GLvoid *)header.vertices;
        
        const GLvoid *offsetVertex   = (const GLvoid *)offsetof(StaticMesh::Vertex, position);
        const GLvoid *offsetTexCoord = (const GLvoid *)offsetof(StaticMesh::Vertex, texCoord);
        const GLvoid *offsetColor    = (const GLvoid *)offsetof(StaticMesh::Vertex, color);
        
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        
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
        
        CHECK_GL_ERROR();
    }
    
    StaticMeshVAO::~StaticMeshVAO()
    {
        glDeleteVertexArrays(1, &_vao);
    }

} // namespace PinkTopaz