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
#include "SDL.h"

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
            throw Exception("Unsupported buffer type %x\n", (int)type);
    }
}

BufferOpenGL::BufferOpenGL(unsigned id,
                           const std::shared_ptr<CommandQueue> &commandQueue,
                           const std::vector<uint8_t> &bufferData,
                           BufferUsage usage,
                           BufferType bufferType)
 : _id(id),
   _vao(0),
   _vbo(0),
   _usage(getUsageEnum(usage)),
   _bufferType(bufferType),
   _commandQueue(commandQueue)
{
    _commandQueue->enqueue(_id, [=]{
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "BufferOpenGL::BufferOpenGL 1");
        internalCreate(bufferData.size(), (void *)&bufferData[0]);
    });
}

BufferOpenGL::BufferOpenGL(unsigned id,
                           const std::shared_ptr<CommandQueue> &commandQueue,
                           size_t bufferSize,
                           const void *bufferData,
                           BufferUsage usage,
                           BufferType bufferType)
: _id(id),
   _vao(0),
   _vbo(0),
   _usage(getUsageEnum(usage)),
   _bufferType(bufferType),
   _commandQueue(commandQueue)
{
    std::vector<uint8_t> wrappedData(bufferSize);
    memcpy(&wrappedData[0], bufferData, bufferSize);
    _commandQueue->enqueue(_id, [data{std::move(wrappedData)}, this]{
        internalCreate(data.size(), (void *)&data[0]);
    });
}

BufferOpenGL::BufferOpenGL(unsigned id,
                           const std::shared_ptr<CommandQueue> &commandQueue,
                           size_t bufferSize,
                           BufferUsage usage,
                           BufferType bufferType)
 : _id(id),
   _vao(0),
   _vbo(0),
   _usage(getUsageEnum(usage)),
   _bufferType(bufferType),
   _commandQueue(commandQueue)
{
    _commandQueue->enqueue(_id, [=]{
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "BufferOpenGL::BufferOpenGL 2");
        internalCreate(bufferSize, nullptr);
    });
}

void BufferOpenGL::internalCreate(size_t bufferSize, const void *bufferData)
{
    CHECK_GL_ERROR();
    
    GLuint vao = 0, vbo = 0;
    GLenum target = getTargetEnum();
    
    if (_bufferType == ArrayBuffer) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        CHECK_GL_ERROR();
    }
    
    glGenBuffers(1, &vbo);
    CHECK_GL_ERROR();
    glBindBuffer(target, vbo);
    CHECK_GL_ERROR();
    glBufferData(target, bufferSize, bufferData, _usage);
    CHECK_GL_ERROR();
    glBindBuffer(target, 0);
    CHECK_GL_ERROR();
    
    if (_bufferType == ArrayBuffer) {
        glBindVertexArray(0);
        CHECK_GL_ERROR();
    }
    
    _vao = vao;
    _vbo = vbo;
    
    CHECK_GL_ERROR();
}

void BufferOpenGL::internalReplace(size_t bufferSize, const void *bufferData)
{
    GLuint vao = _vao;
    GLuint vbo = _vbo;
    GLenum usage = _usage;
    GLenum target = getTargetEnum();
    
    if (_bufferType == ArrayBuffer) {
        glBindVertexArray(vao);
    }
    
    glBindBuffer(target, vbo);
    glBufferData(target, bufferSize, nullptr, usage); // Orphan the buffer.
    glBufferData(target, bufferSize, bufferData, usage);
    glBindBuffer(target, 0);
    
    if (_bufferType == ArrayBuffer) {
        glBindVertexArray(0);
    }
    
    CHECK_GL_ERROR();
}

void BufferOpenGL::replace(const std::vector<uint8_t> &wrappedData)
{
    _commandQueue->enqueue(_id, [wrappedData, this]{
        size_t n = wrappedData.size();
        const void *p = (const void *)&wrappedData[0];
        internalReplace(n, p);
    });
}

void BufferOpenGL::replace(std::vector<uint8_t> &&wrappedData)
{
    _commandQueue->enqueue(_id, [data{std::move(wrappedData)}, this]{
        size_t n = data.size();
        const void *p = (const void *)&data[0];
        internalReplace(n, p);
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
    const unsigned id = _id;
    const GLuint vao = _vao;
    const GLuint vbo = _vbo;
    
    _commandQueue->cancel(id);
    
    _commandQueue->enqueue(0, [vao, vbo]{
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "BufferOpenGL::~BufferOpenGL");
        
        if (vbo) {
            glDeleteBuffers(1, &vbo);
        }
        
        if (vao) {
            glDeleteVertexArrays(1, &vao);
        }
    });
}
