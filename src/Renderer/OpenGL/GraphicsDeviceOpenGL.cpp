//
//  GraphicsDeviceOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
#include "Renderer/OpenGL/CommandEncoderOpenGL.hpp"
#include "Renderer/OpenGL/ShaderOpenGL.hpp"
#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/TextureSamplerOpenGL.hpp"
#include "Renderer/OpenGL/BufferOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Renderer/OpenGL/opengl.hpp"
#include "SDLException.hpp"
#include "FileUtilities.hpp"

#include <vector>

static constexpr unsigned FIRST_ID = 2;

GraphicsDeviceOpenGL::GraphicsDeviceOpenGL(std::shared_ptr<spdlog::logger> log,
                                           SDL_Window &window)
 : _nextId(FIRST_ID),
   _window(window),
   _commandQueue(std::make_shared<CommandQueue>(log))
{
    // VMWare provides OpenGL 3.3. So, this is our minimum OpenGL version.
    // The next lowest is macOS, which provides 4.1.
    constexpr int desiredMajor = 3;
    constexpr int desiredMinor = 3;
    
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, desiredMajor) != 0) {
        throw SDLException("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, desiredMajor={})", desiredMajor);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, desiredMinor) != 0) {
        throw SDLException("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, desiredMinor={})", desiredMinor);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
        throw SDLException("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, SDL_GL_CONTEXT_PROFILE_CORE)");
    }
    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0) {
        throw SDLException("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1)");
    }
    if (SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1) != 0) {
        throw SDLException("SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1)");
    }
        
    _glContext = SDL_GL_CreateContext(&_window);
    if (!_glContext) {
        throw SDLException("SDL_GL_CreateContext failed");
    }
    
    // Check the OpenGL version and log an error if it's not supported.
    // But we'll try to run anyway.
    {
        int major = 0;
        int minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        _log->info("OpenGL version is {}.{}", major, minor);
        
        if ((major < desiredMajor) || ((major == desiredMajor) && (minor < desiredMinor))) {
            _log->error("This application requires at least "
                        "OpenGL {}.{} to run.", desiredMajor, desiredMinor);
        }
    }
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        throw GraphicsDeviceInitFailureOpenGLException("glewInit failed: {}", glewGetErrorString(err));
    }
    
    if (SDL_GL_SetSwapInterval(1) != 0) {
        throw SDLException("SDL_GL_SetSwapInterval failed");
    }
    
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    
    // Determine the maximum sizes of buffers.
    int maxUniformBufferSize = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    _maxBufferSizes[UniformBuffer] = (size_t)maxUniformBufferSize;
    _maxBufferSizes[ArrayBuffer] = std::numeric_limits<std::size_t>::max();
    _maxBufferSizes[IndexBuffer] = std::numeric_limits<std::size_t>::max();
    
    CHECK_GL_ERROR();
}

GraphicsDeviceOpenGL::~GraphicsDeviceOpenGL()
{
    SDL_GL_DeleteContext(_glContext);
}

std::shared_ptr<CommandEncoder> GraphicsDeviceOpenGL::encoder(const RenderPassDescriptor &desc)
{
    auto encoder = std::make_shared<CommandEncoderOpenGL>(_log, nextId(), _commandQueue, desc);
    return std::dynamic_pointer_cast<CommandEncoder>(encoder);
}

void GraphicsDeviceOpenGL::swapBuffers()
{
    _commandQueue->execute();
    SDL_GL_SwapWindow(&_window);
}

std::shared_ptr<Shader>
GraphicsDeviceOpenGL::makeShader(const VertexFormat &vertexFormat,
                                 const std::string &vertexProgramName,
                                 const std::string &fragmentProgramName,
                                 bool blending)
{
    const boost::filesystem::path vertexProgramSourceFileName(vertexProgramName + ".glsl");
    std::string vertexShaderSource = stringFromFileContents(vertexProgramSourceFileName);
    
    const boost::filesystem::path fragmentProgramSourceFileName(fragmentProgramName + ".glsl");
    std::string fragmentShaderSource = stringFromFileContents(fragmentProgramSourceFileName);
    
    auto shader = std::make_shared<ShaderOpenGL>(nextId(),
                                                 _commandQueue,
                                                 vertexFormat,
                                                 vertexShaderSource,
                                                 fragmentShaderSource,
                                                 blending);
    return std::dynamic_pointer_cast<Shader>(shader);
}

std::shared_ptr<Texture> GraphicsDeviceOpenGL::makeTexture(const TextureDescriptor &desc, const void *data)
{
    auto texture = std::make_shared<TextureOpenGL>(nextId(), _commandQueue, desc, data);
    return std::dynamic_pointer_cast<Texture>(texture);
}

std::shared_ptr<Texture> GraphicsDeviceOpenGL::makeTexture(const TextureDescriptor &desc, const std::vector<uint8_t> &data)
{
    auto texture = std::make_shared<TextureOpenGL>(nextId(), _commandQueue, desc, data);
    return std::dynamic_pointer_cast<Texture>(texture);
}

std::shared_ptr<TextureSampler>
GraphicsDeviceOpenGL::makeTextureSampler(const TextureSamplerDescriptor &desc)
{
    auto sampler = std::make_shared<TextureSamplerOpenGL>(nextId(), _commandQueue, desc);
    return std::dynamic_pointer_cast<TextureSampler>(sampler);
}

std::shared_ptr<Buffer>
GraphicsDeviceOpenGL::makeBuffer(const std::vector<uint8_t> &data,
                                 BufferUsage usage,
                                 BufferType bufferType)
{
    auto buffer = std::make_shared<BufferOpenGL>(nextId(), _commandQueue, data,
                                                 usage, bufferType);
    return std::dynamic_pointer_cast<Buffer>(buffer);
}

std::shared_ptr<Buffer>
GraphicsDeviceOpenGL::makeBuffer(size_t bufferSize,
                                 const void *bufferData,
                                 BufferUsage usage,
                                 BufferType bufferType)
{
    auto buffer = std::make_shared<BufferOpenGL>(nextId(), _commandQueue,
                                                 bufferSize, bufferData, usage,
                                                 bufferType);
    return std::dynamic_pointer_cast<Buffer>(buffer);
}

std::shared_ptr<Buffer>
GraphicsDeviceOpenGL::makeBuffer(size_t bufferSize,
                                 BufferUsage usage,
                                 BufferType bufferType)
{
    auto buffer = std::make_shared<BufferOpenGL>(nextId(), _commandQueue,
                                                 bufferSize, usage, bufferType);
    return std::dynamic_pointer_cast<Buffer>(buffer);
}

size_t GraphicsDeviceOpenGL::getMaxBufferSize(BufferType bufferType)
{
    return _maxBufferSizes[bufferType];
}

void GraphicsDeviceOpenGL::windowSizeChanged() {}

const glm::mat4& GraphicsDeviceOpenGL::getProjectionAdjustMatrix() const
{
    static const glm::mat4 identity;
    return identity;
}

unsigned GraphicsDeviceOpenGL::nextId()
{
    return _nextId++;
}
