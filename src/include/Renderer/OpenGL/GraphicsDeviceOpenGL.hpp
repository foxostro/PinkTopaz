//
//  GraphicsDeviceOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef GraphicsDeviceOpenGL_hpp
#define GraphicsDeviceOpenGL_hpp


#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

#include <queue>
#include <memory>
#include <atomic>
#include <map>
#include "SDL.h"
#include <spdlog/spdlog.h>


class GraphicsDeviceOpenGL : public GraphicsDevice
{
public:
    GraphicsDeviceOpenGL(std::shared_ptr<spdlog::logger> log,
                         SDL_Window &window);
    
    virtual ~GraphicsDeviceOpenGL();
    
    // Call this to begin a frame. It returns a command encoder which can be
    // used to encoder graphics commands for submission to the graphics
    // at the end of the frame.
    std::shared_ptr<CommandEncoder>
    encoder(const RenderPassDescriptor &desc) override;
    
    // Flushes commands and swaps buffers. Some underlying graphics APIs
    // have restrictions about which threads they can be used on. So, it's
    // the caller's responsibility call only from method on the main thread.
    void swapBuffers() override;
    
    // Create a new shader using the specified vertex and fragment programs.
    std::shared_ptr<Shader>
    makeShader(const VertexFormat &vertexFormat,
               const std::string &vertexProgramName,
               const std::string &fragmentProgramName,
               bool blending) override;
    
    // Creates a new texture from the specified descriptor and data.
    std::shared_ptr<Texture> makeTexture(const TextureDescriptor &desc,
                                         const void *data) override;
    
    // Creates a new texture from the specified descriptor and data.
    std::shared_ptr<Texture>
    makeTexture(const TextureDescriptor &desc,
                const std::vector<uint8_t> &data) override;
    
    // Creates a new texture sampler from the specified descriptor.
    std::shared_ptr<TextureSampler>
    makeTextureSampler(const TextureSamplerDescriptor &desc) override;
    
    // Creates a new GPU buffer object.
    std::shared_ptr<Buffer>
    makeBuffer(const std::vector<uint8_t> &bufferData,
               BufferUsage usage,
               BufferType bufferType) override;
    
    // Creates a new GPU buffer object.
    std::shared_ptr<Buffer>
    makeBuffer(size_t size,
               const void *data,
               BufferUsage usage,
               BufferType bufferType) override;
    
    // Creates a new GPU buffer object with undefined contents.
    std::shared_ptr<Buffer>
    makeBuffer(size_t size,
               BufferUsage usage,
               BufferType bufferType) override;
    
    // Gets the maximum size (in bytes) of a buffer of the specified type.
    size_t getMaxBufferSize(BufferType bufferType) override;
    
    // Call this when the window size changes. This provides the opportunity
    // to update the underlying context or layers as needed.
    void windowSizeChanged() override;
    
    // Returns a matrix that you should left-multiply against the projection
    // matrix before sending to the shader. This corrects for differences
    // in expected normalized device coordinates between different graphics
    // APIs.
    //
    // Different expect to use different coordinate spaces for normalized
    // device coordinates. For example, OpenGL uses an NDC space that's a
    // cube [-1,+1] x [-1,+1] x [-1,+1] and Metal uses an NDC space that's
    // a cuboid [-1,+1] x [-1,+1] x [0,+1].
    const glm::mat4& getProjectionAdjustMatrix() const override;
    
    // Get the name of this type of graphics device.
    // Mostly useful for debugging.
    std::string getName() const override
    {
        return "OpenGL";
    }
    
private:
    unsigned nextId();
    
    std::atomic<unsigned> _nextId;
    SDL_Window &_window;
    SDL_GLContext _glContext;
    std::shared_ptr<CommandQueue> _commandQueue;
    std::map<BufferType, size_t> _maxBufferSizes;
    std::shared_ptr<spdlog::logger> _log;
};

#endif /* GraphicsDeviceOpenGL_hpp */
