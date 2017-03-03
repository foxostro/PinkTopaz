//
//  GraphicsDevice.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/20/17.
//
//

#ifndef GraphicsDevice_hpp
#define GraphicsDevice_hpp

#include <memory>
#include <string>

#include "Renderer/CommandEncoder.hpp"
#include "Renderer/StaticMesh.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexFormat.hpp"
#include "Renderer/RenderPassDescriptor.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"

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
        // This call is garaunteed thread-safe. Commands will be submitted to
        // GPU in the order of call to this method; however, commands may not
        // be flushed until the next call to swapBuffers().
        virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) = 0;
        
        // Flushes commands and swaps buffers. Some underlying graphics APIs
        // have restrictions about which threads they can be used on. So, it's
        // the caller's responsibility call only from method on the main thread.
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
        makeBuffer(const VertexFormat &format,
                   const std::vector<uint8_t> &bufferData,
                   size_t elementCount,
                   BufferUsage usage) = 0;
        
        // Creates a new GPU buffer object with indefined contents.
        virtual std::shared_ptr<Buffer>
        makeBuffer(const VertexFormat &format,
                   size_t size,
                   size_t elementCount,
                   BufferUsage usage) = 0;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* GraphicsDevice_hpp */
