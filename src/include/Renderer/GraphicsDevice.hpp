//
//  GraphicsDevice.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/20/17.
//
//

#ifndef GraphicsDevice_hpp
#define GraphicsDevice_hpp

#include "Renderer/CommandEncoder.hpp"
#include "Renderer/StaticMesh.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexFormat.hpp"
#include "Renderer/RenderPassDescriptor.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"

#include <memory>
#include <string>

#include "SDL.h"

namespace PinkTopaz::Renderer {

    // This is a thin abstraction layer over the graphics API. Instantiate a
    // concrete GraphicsDevice sub-class that implements an API such as OpenGL.
    class GraphicsDevice
    {
    public:
        GraphicsDevice() {}
        virtual ~GraphicsDevice() {}
        
        // Call this to begin a frame. It returns a command encoder which can be
        // used to encoder graphics commands for submission to the graphics
        // at the end of the frame.
        virtual std::shared_ptr<CommandEncoder>
        encoder(const RenderPassDescriptor &descriptor) = 0;

        // Call this to submit commands to the graphics device.
        virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) = 0;
        
        // Flushes commands and swaps buffers.
        virtual void swapBuffers() = 0;
        
        // Create a new shader using the specified vertex and fragment programs.
        virtual std::shared_ptr<Shader>
        makeShader(const std::string &vertexProgramName,
                   const std::string &fragmentProgramName) = 0;
        
        // Creates a new texture from the specified descriptor and data.
        virtual std::shared_ptr<Texture>
        makeTexture(const TextureDescriptor &desc,
                    const void *data) = 0;
        
        // Creates a new texture from the specified descriptor and data.
        virtual std::shared_ptr<Texture>
        makeTexture(const TextureDescriptor &desc,
                    const std::vector<uint8_t> &data) = 0;
        
        // Creates a new texture sampler from the specified descriptor.
        virtual std::shared_ptr<TextureSampler>
        makeTextureSampler(const TextureSamplerDescriptor &desc) = 0;
        
        // Creates a new GPU buffer object.
        virtual std::shared_ptr<Buffer>
        makeBuffer(const std::vector<uint8_t> &bufferData,
                   BufferUsage usage,
                   BufferType bufferType) = 0;
        
        // Creates a new GPU buffer object.
        virtual std::shared_ptr<Buffer>
        makeBuffer(size_t size,
                   const void *data,
                   BufferUsage usage,
                   BufferType bufferType) = 0;
        
        // Creates a new GPU buffer object with undefined contents.
        virtual std::shared_ptr<Buffer>
        makeBuffer(size_t size,
                   BufferUsage usage,
                   BufferType bufferType) = 0;
        
        // Call this when the window size changes. This provides the opportunity
        // to update the underlying context or layers as needed.
        virtual void windowSizeChanged() = 0;
    };
    
    // Create a default graphics device for the system.
    std::shared_ptr<GraphicsDevice> createDefaultGraphicsDevice(SDL_Window &w);
    
} // namespace PinkTopaz::Renderer

#endif /* GraphicsDevice_hpp */
