//
//  BufferOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#include "Renderer/OpenGL/BufferOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"
#include <cassert>

namespace PinkTopaz::Renderer::OpenGL {
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const VertexFormat &format,
                               const std::vector<uint8_t> &bufferData,
                               size_t n,
                               BufferUsage usage)
     : _vao(0),
       _count(n),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            GLenum usageEnum;
            switch (usage)
            {
                case BufferUsageStaticDraw:
                    usageEnum = GL_STATIC_DRAW;
                    break;
                    
                default:
                    throw Exception("Unsupported buffer usage %d in BufferOpenGL\n", (int)usage);
            }
            
            GLuint vao = 0;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            GLuint vbo = 0;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            
            size_t size = bufferData.size();
            const GLvoid *data = (const GLvoid *)bufferData.data();
            glBufferData(GL_ARRAY_BUFFER, size, data, usageEnum);
            
            for (size_t i = 0; i < format.attributes.size(); ++i)
            {
                const AttributeFormat &attr = format.attributes[i];
                
                GLenum typeEnum;
                switch (attr.type)
                {
                    case AttributeTypeFloat:
                        typeEnum = GL_FLOAT;
                        break;
                        
                    case AttributeTypeUnsignedByte:
                        typeEnum = GL_UNSIGNED_BYTE;
                        break;
                        
                    default:
                        throw Exception("Unsupported buffer attribute type %d in BufferOpenGL\n", (int)attr.type);
                }
                
                glVertexAttribPointer(i,
                                      (GLint)attr.size,
                                      typeEnum,
                                      attr.normalized ? GL_TRUE : GL_FALSE,
                                      (GLsizei)attr.stride,
                                      (const GLvoid *)attr.offset);
                glEnableVertexAttribArray(i);
            }

            glBindVertexArray(0);
            glDeleteBuffers(1, &vbo); // Release our reference after the VAO is initialized.
            
            this->_vao = vao;
            
            CHECK_GL_ERROR();
        });
    }
    
    BufferOpenGL::~BufferOpenGL()
    {
        GLuint handle = _vao;
        _commandQueue.enqueue([handle]{
            glDeleteVertexArrays(1, &handle);
        });
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
