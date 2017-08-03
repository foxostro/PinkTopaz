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
#import "Exception.hpp"

#import "SDL_syswm.h"

GraphicsDeviceMetal::GraphicsDeviceMetal(SDL_Window &window)
{
    _pool = [[NSAutoreleasePool alloc] init];
    
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    // Create a Metal layer for displaying rendering results in the window.
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
        _metalLayer.device = device;
        _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        _metalLayer.frame = sdlLayer.frame;
        _metalLayer.framebufferOnly = true;
        
        [sdlLayer addSublayer:_metalLayer];
    }
    
    // Create the depth buffer.
    _depthTexture = nil;
    rebuildDepthTexture();
    
    // Create some state objects for the depth test being ON and OFF.
    {
        MTLDepthStencilDescriptor *depthDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        
        depthDescriptor.depthWriteEnabled = YES;
        depthDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        _depthTestOn = [device newDepthStencilStateWithDescriptor:depthDescriptor];
        
        depthDescriptor.depthWriteEnabled = NO;
        depthDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
        _depthTestOff = [device newDepthStencilStateWithDescriptor:depthDescriptor];
        
        [depthDescriptor release];
    }
    
    // We need a command queue in order to control the GPU.
    _commandQueue = [device newCommandQueue];
    
    // Load the shader library we're going to use.
    {
        NSError *error = nil;
        NSString *libraryName = @"Library.metallib";
        _library = [device newLibraryWithFile:libraryName error:&error];
        if (!_library) {
            NSString *errorDesc = [error localizedDescription];
            throw Exception("Failed to create Metal shader library \"%s\": %s",
                            libraryName.UTF8String, errorDesc.UTF8String);
        }
    }
    
    [device release];
}

GraphicsDeviceMetal::~GraphicsDeviceMetal()
{
    [_metalLayer removeFromSuperlayer];
    
    [_library release];
    [_commandQueue release];
    [_metalLayer release];
    [_depthTexture release];
    [_depthTestOn release];
    [_depthTestOff release];
    [_pool release];
}

std::shared_ptr<CommandEncoder>
GraphicsDeviceMetal::encoder(const RenderPassDescriptor &desc)
{
    id <CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    
    auto encoder = std::make_shared<CommandEncoderMetal>(desc,
                                                         _metalLayer.device,
                                                         _commandQueue,
                                                         drawable,
                                                         _depthTexture,
                                                         _depthTestOn,
                                                         _depthTestOff);
    return std::dynamic_pointer_cast<CommandEncoder>(encoder);
}

void GraphicsDeviceMetal::swapBuffers() {}

std::shared_ptr<Shader>
GraphicsDeviceMetal::makeShader(const VertexFormat &vertexFormat,
                                const std::string &vert,
                                const std::string &frag,
                                bool blending)
{
    auto shader = std::make_shared<ShaderMetal>(vertexFormat,
                                                _metalLayer.device,
                                                _library, vert, frag,
                                                blending);
    return std::dynamic_pointer_cast<Shader>(shader);
}

std::shared_ptr<Texture>
GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                 const void *data)
{
    auto texture = std::make_shared<TextureMetal>(_metalLayer.device,
                                                  desc, data);
    return std::dynamic_pointer_cast<Texture>(texture);
}

std::shared_ptr<Texture>
GraphicsDeviceMetal::makeTexture(const TextureDescriptor &desc,
                                 const std::vector<uint8_t> &data)
{
    return makeTexture(desc, &data[0]);
}

std::shared_ptr<TextureSampler>
GraphicsDeviceMetal::makeTextureSampler(const TextureSamplerDescriptor &desc)
{
    auto sampler = std::make_shared<TextureSamplerMetal>(_metalLayer.device, desc);
    return std::dynamic_pointer_cast<TextureSampler>(sampler);
}

std::shared_ptr<Buffer>
GraphicsDeviceMetal::makeBuffer(const std::vector<uint8_t> &data,
                                BufferUsage usage, BufferType bufferType)
{
    assert(data.size() > 0);
    return makeBuffer(data.size(), &data[0], usage, bufferType);
}

std::shared_ptr<Buffer>
GraphicsDeviceMetal::makeBuffer(size_t size,
                                const void *bufferData,
                                BufferUsage usage,
                                BufferType type)
{
    assert(size > 0);
    auto buffer = std::make_shared<BufferMetal>(_metalLayer.device, size,
                                                bufferData, usage, type);
    return std::dynamic_pointer_cast<Buffer>(buffer);
}

std::shared_ptr<Buffer>
GraphicsDeviceMetal::makeBuffer(size_t size,
                                BufferUsage usage,
                                BufferType bufferType)
{
    auto buffer = std::make_shared<BufferMetal>(_metalLayer.device, size,
                                                usage, bufferType);
    return std::dynamic_pointer_cast<Buffer>(buffer);
}

void GraphicsDeviceMetal::windowSizeChanged()
{
    // Resize the layer when the window resizes.
    _metalLayer.frame = _metalLayer.superlayer.frame;
    rebuildDepthTexture();
}

void GraphicsDeviceMetal::rebuildDepthTexture()
{
    [_depthTexture release];
    
    CGSize drawableSize = _metalLayer.drawableSize;
    MTLTextureDescriptor *descriptor =
    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                       width:drawableSize.width
                                                      height:drawableSize.height
                                                   mipmapped:NO];
    descriptor.storageMode = MTLStorageModePrivate;
    descriptor.usage = MTLTextureUsageRenderTarget;
    _depthTexture = [_metalLayer.device newTextureWithDescriptor:descriptor];
    _depthTexture.label = @"Depth";
}

const glm::mat4& GraphicsDeviceMetal::getProjectionAdjustMatrix() const
{
    static const glm::mat4 adjust(glm::vec4(1.0, 0.0, 0.0, 0.0),
                                  glm::vec4(0.0, 1.0, 0.0, 0.0),
                                  glm::vec4(0.0, 0.0, 0.5, 0.0),
                                  glm::vec4(0.0, 0.0, 0.0, 1.0));
    return adjust;
}
