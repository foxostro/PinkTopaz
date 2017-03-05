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
#include "Renderer/OpenGL/FenceOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include <vector>

namespace PinkTopaz::Renderer::OpenGL {
    
    GraphicsDeviceOpenGL::GraphicsDeviceOpenGL(SDL_Window &window)
     : _window(window)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetSwapInterval(1);
        _glContext = SDL_GL_CreateContext(&_window);
        
        // Check the OpenGL version and log an error if it's not supported.
        // But we'll try to run anyway.
        {
            int major = 0;
            int minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "OpenGL version is %d.%d\n", major, minor);
            
            if (!(major >= 4 && minor >= 1)) {
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "This application requires at least OpenGL 4.1 to run.");
            }
        }
        
        glClearColor(0.2, 0.4, 0.5, 1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        glEnable(GL_CULL_FACE);

        CHECK_GL_ERROR();
    }
    
    GraphicsDeviceOpenGL::~GraphicsDeviceOpenGL()
    {
        SDL_GL_DeleteContext(_glContext);
    }
    
    std::shared_ptr<CommandEncoder> GraphicsDeviceOpenGL::encoder(const RenderPassDescriptor &desc)
    {
        auto encoder = std::make_shared<CommandEncoderOpenGL>(desc);
        return std::dynamic_pointer_cast<CommandEncoder>(encoder);
    }
    
    void GraphicsDeviceOpenGL::submit(const std::shared_ptr<CommandEncoder> &abstractEncoder)
    {
        auto concreteEncoder = std::dynamic_pointer_cast<CommandEncoderOpenGL>(abstractEncoder);
        assert(concreteEncoder);
        _commandQueue.enqueue(concreteEncoder->getCommandQueue());
    }
    
    void GraphicsDeviceOpenGL::swapBuffers()
    {
        CHECK_GL_ERROR();
        _commandQueue.execute();
        SDL_GL_SwapWindow(&_window);
        CHECK_GL_ERROR();
    }
    
    std::shared_ptr<Shader> GraphicsDeviceOpenGL::makeShader(const std::string &vertexProgramName, const std::string &fragmentProgramName)
    {
        std::string vertexProgramSourceFileName = vertexProgramName + ".glsl";
        std::string vertexShaderSource = stringFromFileContents(vertexProgramSourceFileName.c_str());
        
        std::string fragmentProgramSourceFileName = fragmentProgramName + ".glsl";
        std::string fragmentShaderSource = stringFromFileContents(fragmentProgramSourceFileName.c_str());

        auto shader = std::make_shared<ShaderOpenGL>(_commandQueue, vertexShaderSource, fragmentShaderSource);
        return std::dynamic_pointer_cast<Shader>(shader);
    }
    
    std::shared_ptr<Texture> GraphicsDeviceOpenGL::makeTexture(const TextureDescriptor &desc, const void *data)
    {
        auto texture = std::make_shared<TextureOpenGL>(_commandQueue, desc, data);
        return std::dynamic_pointer_cast<Texture>(texture);
    }
    
    std::shared_ptr<Texture> GraphicsDeviceOpenGL::makeTexture(const TextureDescriptor &desc, const std::vector<uint8_t> &data)
    {
        auto texture = std::make_shared<TextureOpenGL>(_commandQueue, desc, data);
        return std::dynamic_pointer_cast<Texture>(texture);
    }
    
    std::shared_ptr<TextureSampler>
    GraphicsDeviceOpenGL::makeTextureSampler(const TextureSamplerDescriptor &desc)
    {
        auto sampler = std::make_shared<TextureSamplerOpenGL>(_commandQueue, desc);
        return std::dynamic_pointer_cast<TextureSampler>(sampler);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceOpenGL::makeBuffer(const VertexFormat &format,
                                     const std::vector<uint8_t> &bufferData,
                                     BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferOpenGL>(_commandQueue,
                                                     format,
                                                     bufferData,
                                                     usage,
                                                     ArrayBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceOpenGL::makeBuffer(const VertexFormat &format,
                                     size_t bufferSize,
                                     BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferOpenGL>(_commandQueue,
                                                     format,
                                                     bufferSize,
                                                     usage,
                                                     ArrayBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceOpenGL::makeUniformBuffer(const std::vector<uint8_t> &data,
                                            BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferOpenGL>(_commandQueue,
                                                     data,
                                                     usage,
                                                     UniformBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceOpenGL::makeUniformBuffer(size_t size, BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferOpenGL>(_commandQueue,
                                                     size,
                                                     usage,
                                                     UniformBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Fence> GraphicsDeviceOpenGL::makeFence()
    {
        auto fence = std::make_shared<FenceOpenGL>(_commandQueue);
        return std::dynamic_pointer_cast<Fence>(fence);
    }
    
    void GraphicsDeviceOpenGL::windowSizeChanged() {}
    
} // namespace PinkTopaz::Renderer::OpenGL
