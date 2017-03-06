//
//  GraphicsDeviceMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef GraphicsDeviceMetal_h
#define GraphicsDeviceMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#import "Renderer/GraphicsDevice.hpp"
#import "Renderer/CommandEncoder.hpp"

#import <queue>
#import <memory>

#import "SDL.h"
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class GraphicsDeviceMetal : public GraphicsDevice
    {
    public:
        GraphicsDeviceMetal(SDL_Window &window);
        virtual ~GraphicsDeviceMetal();
        
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
        std::shared_ptr<Texture>
        makeTexture(const TextureDescriptor &desc,
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
        
        // Call this when the window size changes. This provides the opportunity
        // to update the underlying context or layers as needed.
        void windowSizeChanged() override;
        
    private:
        void rebuildDepthTexture();
        
        CAMetalLayer *_metalLayer;
        id <MTLCommandQueue> _commandQueue;
        id <MTLLibrary> _library;
        id <MTLTexture> _depthTexture;
        id <MTLDepthStencilState> _depthTestOn, _depthTestOff;
        NSAutoreleasePool *_pool;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* GraphicsDeviceMetal_h */
