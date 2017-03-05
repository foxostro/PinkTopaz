//
//  GraphicsDeviceMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#import "Renderer/Metal/GraphicsDeviceMetal.h"
#import "Renderer/Metal/CommandEncoderMetal.h"
#import "Renderer/Metal/ShaderMetal.h"
#import "Renderer/Metal/TextureMetal.h"
#import "Renderer/Metal/TextureSamplerMetal.h"
#import "Renderer/Metal/BufferMetal.h"
#import "Renderer/Metal/FenceMetal.h"
#import "Exception.hpp"

#import "SDL_syswm.h"

namespace PinkTopaz::Renderer::Metal {
    
    GraphicsDeviceMetal::GraphicsDeviceMetal(SDL_Window &window)
    {
        SDL_SysWMinfo windowManagerInfo;
        SDL_VERSION(&windowManagerInfo.version);
        SDL_GetWindowWMInfo(&window, &windowManagerInfo);
        
        // Create a metal layer and add it to the view that SDL created.
        NSView *sdlView = windowManagerInfo.info.cocoa.window.contentView;
        sdlView.wantsLayer = YES;
        CALayer *sdlLayer = sdlView.layer;
        
        CGFloat contentsScale = sdlLayer.contentsScale;
        NSSize layerSize = sdlLayer.frame.size;
        
        _metalLayer = [[CAMetalLayer layer] retain];
        _metalLayer.contentsScale = contentsScale;
        _metalLayer.drawableSize = NSMakeSize(layerSize.width * contentsScale,
                                             layerSize.height * contentsScale);
        _metalLayer.device = MTLCreateSystemDefaultDevice();
        _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        _metalLayer.frame = sdlLayer.frame;
        _metalLayer.framebufferOnly = true;
        
        [sdlLayer addSublayer:_metalLayer];
        
        // We need a command queue in order to control the GPU.
        _commandQueue = [_metalLayer.device newCommandQueue];
        
        // Load the shader library.
        NSError *error = nil;
        NSString *libraryName = @"Library.metallib";
        _library = [_metalLayer.device newLibraryWithFile:libraryName error:&error];
        if (!_library) {
            NSString *errorDesc = [error localizedDescription];
            throw Exception("Failed to create Metal shader library \"%s\": %s",
                            libraryName.UTF8String, errorDesc.UTF8String);
        }
    }
    
    GraphicsDeviceMetal::~GraphicsDeviceMetal()
    {
        [_metalLayer removeFromSuperlayer];
        
        [_library release];
        [_commandQueue release];
        [_metalLayer release];
    }
    
    std::shared_ptr<CommandEncoder>
    GraphicsDeviceMetal::encoder(const RenderPassDescriptor &desc)
    {
        id <CAMetalDrawable> drawable = [_metalLayer nextDrawable];
        
        auto encoder = std::make_shared<CommandEncoderMetal>(desc, _commandQueue, drawable);
        return std::dynamic_pointer_cast<CommandEncoder>(encoder);
    }
    
    void GraphicsDeviceMetal::submit(const std::shared_ptr<CommandEncoder> &abstractEncoder)
    {
        auto concreteEncoder = std::dynamic_pointer_cast<CommandEncoderMetal>(abstractEncoder);
        assert(concreteEncoder);
        concreteEncoder->onSubmit();
    }
    
    void GraphicsDeviceMetal::swapBuffers() {}
    
    std::shared_ptr<Shader>
    GraphicsDeviceMetal::makeShader(const std::string &vert,
                                    const std::string &frag)
    {
        auto shader = std::make_shared<ShaderMetal>(_metalLayer.device,
                                                    _library, vert, frag);
        return std::dynamic_pointer_cast<Shader>(shader);
    }
    
    std::shared_ptr<Texture>
    GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                     const void *data)
    {
        auto texture = std::make_shared<TextureMetal>(desc, data);
        return std::dynamic_pointer_cast<Texture>(texture);
    }
    
    std::shared_ptr<Texture>
    GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                     const std::vector<uint8_t> &data)
    {
        auto texture = std::make_shared<TextureMetal>(desc, data);
        return std::dynamic_pointer_cast<Texture>(texture);
    }
    
    std::shared_ptr<TextureSampler>
    GraphicsDeviceMetal::makeTextureSampler(const TextureSamplerDescriptor &d)
    {
        auto sampler = std::make_shared<TextureSamplerMetal>(d);
        return std::dynamic_pointer_cast<TextureSampler>(sampler);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(const std::vector<uint8_t> &bufferData,
                                    BufferUsage usage,
                                    BufferType bufferType)
    {
        auto buffer = std::make_shared<BufferMetal>(_metalLayer.device,
                                                    bufferData, usage,
                                                    bufferType);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(size_t bufferSize,
                                    const void *bufferData,
                                    BufferUsage usage,
                                    BufferType bufferType)
    {
        auto buffer = std::make_shared<BufferMetal>(_metalLayer.device,
                                                    bufferSize, bufferData,
                                                    usage, bufferType);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(size_t bufferSize,
                                    BufferUsage usage,
                                    BufferType bufferType)
    {
        auto buffer = std::make_shared<BufferMetal>(_metalLayer.device,
                                                    bufferSize, usage,
                                                    bufferType);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Fence> GraphicsDeviceMetal::makeFence()
    {
        // Note that MTLFence is currently unavailable on macOS.
        // TODO: find a different way to measure frame times.
        return std::dynamic_pointer_cast<Fence>(std::make_shared<FenceMetal>());
    }
    
    void GraphicsDeviceMetal::windowSizeChanged()
    {
        // Resize the layer when the window resizes.
        _metalLayer.frame = _metalLayer.superlayer.frame;
    }
    
} // namespace PinkTopaz::Renderer::Metal
