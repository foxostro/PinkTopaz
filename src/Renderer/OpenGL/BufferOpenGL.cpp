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

namespace PinkTopaz::Renderer::OpenGL {
    
    static GLenum getUsageEnum(BufferUsage usage)
    {
        switch (usage)
        {
            case StaticDraw:    return GL_STATIC_DRAW;
            case DynamicDraw:   return GL_DYNAMIC_DRAW;
                
            default:
                throw Exception("Unsupported buffer usage %d\n", (int)usage);
        }
    }
    
    GLenum getBufferTypeEnum(BufferType type)
    {
        switch (type)
        {
            case ArrayBuffer:   return GL_ARRAY_BUFFER;
            case UniformBuffer: return GL_UNIFORM_BUFFER;
                
            default:
                throw Exception("Unsupported buffer type %d\n", (int)type);
        }
    }
    
    BufferOpenGL::BufferOpenGL(const std::vector<uint8_t> &bufferData,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType)
    {
        internalCreate(bufferData.size(), (void *)&bufferData[0]);
    }
    
    BufferOpenGL::BufferOpenGL(size_t bufferSize,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType)
    {
        internalCreate(bufferSize, nullptr);
    }
    
    void BufferOpenGL::internalCreate(size_t bufferSize, void *bufferData)
    {
        GLuint vao = 0, vbo = 0;
        GLenum target = getTargetEnum();
        
        if (_bufferType == ArrayBuffer) {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }
        
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferSize, bufferData, _usage);
        glBindBuffer(target, 0);
        
        if (_bufferType == ArrayBuffer) {
            glBindVertexArray(0);
        }
        
        _vao = vao;
        _vbo = vbo;
        
        CHECK_GL_ERROR();
    }
    
    void BufferOpenGL::internalReplace(const void *p, size_t n)
    {
        GLuint vao = _vao;
        GLuint vbo = _vbo;
        GLenum usage = _usage;
        GLenum target = getTargetEnum();
        
        if (_bufferType == ArrayBuffer) {
            glBindVertexArray(vao);
        }
        
        glBindBuffer(target, vbo);
        glBufferData(target, n, nullptr, usage); // Orphan the buffer.
        glBufferData(target, n, p, usage);
        glBindBuffer(target, 0);
        
        if (_bufferType == ArrayBuffer) {
            glBindVertexArray(0);
        }
        
        CHECK_GL_ERROR();
    }
    
    void BufferOpenGL::replace(const std::vector<uint8_t> &wrappedData)
    {
        size_t n = wrappedData.size();
        const void *p = (const void *)&wrappedData[0];
        internalReplace(p, n);
    }
    
    void BufferOpenGL::replace(std::vector<uint8_t> &&data)
    {
        size_t n = data.size();
        const void *p = (const void *)&data[0];
        internalReplace(p, n);
    }
    
    void BufferOpenGL::replace(size_t size, const void *data)
    {
        std::vector<uint8_t> wrappedData(size);
        memcpy(&wrappedData[0], data, size);
        replace(std::move(wrappedData));
    }
    
    BufferOpenGL::~BufferOpenGL()
    {
        GLuint vao = _vao, vbo = _vbo;
        
        if (vbo) {
            glDeleteBuffers(1, &vbo);
        }
        
        if (vao) {
            glDeleteVertexArrays(1, &vao);
        }
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
