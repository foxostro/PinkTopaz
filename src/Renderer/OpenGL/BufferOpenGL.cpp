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
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const VertexFormat &format,
                               const std::vector<uint8_t> &bufferData,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(&format, bufferData.size(), (void *)&bufferData[0]);
        });
    }
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const VertexFormat &format,
                               size_t bufferSize,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(&format, bufferSize, nullptr);
        });
    }
    
    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               size_t bufferSize,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(nullptr, bufferSize, nullptr);
        });
    }

    BufferOpenGL::BufferOpenGL(CommandQueue &queue,
                               const std::vector<uint8_t> &bufferData,
                               BufferUsage usage,
                               BufferType bufferType)
     : _vao(0),
       _vbo(0),
       _usage(getUsageEnum(usage)),
       _bufferType(bufferType),
       _commandQueue(queue)
    {
        _commandQueue.enqueue([=]{
            internalCreate(nullptr, bufferData.size(), (void *)&bufferData[0]);
        });
    }
    
    void BufferOpenGL::setupVertexAttributes(const VertexFormat &format)
    {
        // We're already holding the lock coming into this method.
        // Buffers are already bound coming into this method.
        
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
        }
    }
    
    void BufferOpenGL::internalCreate(const VertexFormat *pformat,
                                      size_t bufferSize,
                                      void *bufferData)
    {
        std::lock_guard<std::mutex> lock(_lock);
        
        GLuint vao = 0, vbo = 0;
        GLenum target = getTargetEnum();
        
        if (_bufferType == ArrayBuffer) {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }
        
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferSize, bufferData, _usage);
        
        if (pformat) {
            const VertexFormat &format = *pformat;
            setupVertexAttributes(format);
        }
        
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
        std::lock_guard<std::mutex> lock(_lock);
        
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
        _commandQueue.enqueue([wrappedData, this]{
            size_t n = wrappedData.size();
            const void *p = (const void *)&wrappedData[0];
            internalReplace(p, n);
        });
    }
    
    void BufferOpenGL::replace(std::vector<uint8_t> &&wrappedData)
    {
        _commandQueue.enqueue([data{std::move(wrappedData)}, this]{
            size_t n = data.size();
            const void *p = (const void *)&data[0];
            internalReplace(p, n);
        });
    }
    
    void BufferOpenGL::replace(size_t size, const void *data)
    {
        std::vector<uint8_t> wrappedData(size);
        memcpy(&wrappedData[0], data, size);
        replace(std::move(wrappedData));
    }
    
    BufferOpenGL::~BufferOpenGL()
    {
        GLuint vao, vbo;
        
        {
            std::lock_guard<std::mutex> lock(_lock);
            vao = _vao;
            vbo = _vbo;
        }
        
        _commandQueue.enqueue([vao, vbo]{
            if (vbo) {
                glDeleteBuffers(1, &vbo);
            }
            
            if (vao) {
                glDeleteVertexArrays(1, &vao);
            }
        });
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
