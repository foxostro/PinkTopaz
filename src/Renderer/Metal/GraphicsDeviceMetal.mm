//
//  GraphicsDeviceMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#import "Renderer/Metal/GraphicsDeviceMetal.h"
#import "Exception.hpp"

#import "SDL_syswm.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

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
        
        CAMetalLayer *metalLayer = [CAMetalLayer layer];
        metalLayer.contentsScale = contentsScale;
        metalLayer.drawableSize = NSMakeSize(layerSize.width * contentsScale,
                                             layerSize.height * contentsScale);
        metalLayer.device = MTLCreateSystemDefaultDevice();
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.frame = sdlLayer.frame;
        metalLayer.framebufferOnly = true;
        
        [sdlLayer addSublayer:metalLayer];
        
        _metalLayer = [metalLayer retain];
    }
    
    GraphicsDeviceMetal::~GraphicsDeviceMetal()
    {
        [_metalLayer removeFromSuperlayer];
        [_metalLayer release];
        _metalLayer = nil;
    }
    
    std::shared_ptr<CommandEncoder>
    GraphicsDeviceMetal::encoder(const RenderPassDescriptor &desc)
    {
        throw Exception("unimplemented");
    }
    
    void GraphicsDeviceMetal::submit(const std::shared_ptr<CommandEncoder> &enc)
    {
        throw Exception("unimplemented");
    }
    
    void GraphicsDeviceMetal::swapBuffers()
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Shader>
    GraphicsDeviceMetal::makeShader(const std::string &vertexProgramName,
                                    const std::string &fragmentProgramName)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Texture>
    GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                     const void *data)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Texture>
    GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                     const std::vector<uint8_t> &data)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<TextureSampler>
    GraphicsDeviceMetal::makeTextureSampler(const TextureSamplerDescriptor &desc)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(const VertexFormat &format,
                                    const std::vector<uint8_t> &bufferData,
                                    size_t elementCount,
                                    BufferUsage usage)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeBuffer(const VertexFormat &format,
                                    size_t bufferSize,
                                    size_t elementCount,
                                    BufferUsage usage)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeUniformBuffer(const std::vector<uint8_t> &data,
                                           BufferUsage usage)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Buffer>
    GraphicsDeviceMetal::makeUniformBuffer(size_t size, BufferUsage usage)
    {
        throw Exception("unimplemented");
    }
    
    std::shared_ptr<Fence> GraphicsDeviceMetal::makeFence()
    {
        throw Exception("unimplemented");
    }
    
} // namespace PinkTopaz::Renderer::Metal
