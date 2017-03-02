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
    
    static GLenum getUsageEnum(BufferUsage usage)
    {
        switch (usage)
        {
            case StaticDraw:    return GL_STATIC_DRAW;
            case DynamicDraw:   return GL_DYNAMIC_DRAW;
            case StreamDraw:    return GL_STREAM_DRAW;
                
            default:
                throw Exception("Unsupported buffer usage %d\n", (int)usage);
        }
    }
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const VertexFormat &format,
                               const std::vector<uint8_t> &bufferData,
                               size_t elementCount,
                               BufferUsage usage)
     : _vao(0),
       _vbo(0),
       _count(elementCount),
       _usage(getUsageEnum(usage)),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(format, bufferData.size(), (void *)&bufferData[0]);
        });
    }
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const VertexFormat &format,
                               size_t bufferSize,
                               size_t elementCount,
                               BufferUsage usage)
     : _vao(0),
       _vbo(0),
       _count(elementCount),
       _usage(getUsageEnum(usage)),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(format, bufferSize, nullptr);
        });
    }
    
    void BufferOpenGL::internalCreate(const VertexFormat &format,
                                      size_t bufferSize,
                                      void *bufferData)
    {
        GLuint vao, vbo;
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, _usage);
        
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
            
            glVertexAttribPointer((GLuint)i,
                                  (GLint)attr.size,
                                  typeEnum,
                                  attr.normalized ? GL_TRUE : GL_FALSE,
                                  (GLsizei)attr.stride,
                                  (const GLvoid *)attr.offset);
            glEnableVertexAttribArray((GLuint)i);
            
            lock.lock();
            _vao = vao;
            _vbo = vbo;
            lock.unlock();
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        CHECK_GL_ERROR();
    }
    
    void BufferOpenGL::replace(size_t size, const void *data)
    {
        std::vector<uint8_t> wrappedData(size);
        memcpy(&wrappedData[0], data, size);

        _commandQueue.enqueue([=]{
            lock.lock();
            GLuint vao = _vao;
            GLuint vbo = _vbo;
            lock.unlock();
            
            size_t n = wrappedData.size();
            const void *p = (const void *)&wrappedData[0];
            
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, n, p, _usage);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            CHECK_GL_ERROR();
        });
    }
    
    BufferOpenGL::~BufferOpenGL()
    {
        lock.lock();
        GLuint vao = _vao;
        GLuint vbo = _vbo;
        lock.unlock();

        _commandQueue.enqueue([vao, vbo]{
            glDeleteBuffers(1, &vbo);
            glDeleteVertexArrays(1, &vao);
        });
    }
    
} // namespace PinkTopaz::Renderer::OpenGL