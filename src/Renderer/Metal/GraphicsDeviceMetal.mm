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
        
        NSArray <id<MTLDevice>> *devices = MTLCopyAllDevices();
        NSLog(@"devices: %@", devices);
        
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
        
        _commandQueue = [_metalLayer.device newCommandQueue];
    }
    
    GraphicsDeviceMetal::~GraphicsDeviceMetal()
    {
        [_commandQueue release];
        
        [_metalLayer removeFromSuperlayer];
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
        auto shader = std::make_shared<ShaderMetal>(vert, frag);
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
    GraphicsDeviceMetal::makeBuffer(const VertexFormat &format,
                                    const std::vector<uint8_t> &bufferData,
                                    size_t elementCount,
                                    BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferMetal>(format, bufferData,
                                                    elementCount, usage,
                                                    ArrayBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(const VertexFormat &format,
                                    size_t bufferSize,
                                    size_t elementCount,
                                    BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferMetal>(format, bufferSize,
                                                    elementCount, usage,
                                                    ArrayBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeUniformBuffer(const std::vector<uint8_t> &data,
                                           BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferMetal>(data, usage, UniformBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeUniformBuffer(size_t size, BufferUsage usage)
    {
        auto buffer = std::make_shared<BufferMetal>(size, usage, UniformBuffer);
        return std::dynamic_pointer_cast<Buffer>(buffer);
    }
    
    std::shared_ptr<Fence> GraphicsDeviceMetal::makeFence()
    {
        // Note that MTLFence is currently unavailable on macOS.
        return std::dynamic_pointer_cast<Fence>(std::make_shared<FenceMetal>());
    }
    
    void GraphicsDeviceMetal::windowSizeChanged()
    {
        // Resize the layer when the window resizes.
        _metalLayer.frame = _metalLayer.superlayer.frame;
    }
    
} // namespace PinkTopaz::Renderer::Metal
