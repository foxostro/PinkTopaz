//
//  CommandEncoderMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#import "Renderer/Metal/CommandEncoderMetal.h"
#import "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {
    
    CommandEncoderMetal::CommandEncoderMetal(const RenderPassDescriptor &desc,
                                             id <MTLCommandQueue> commandQueue,
                                             id <CAMetalDrawable> drawable)
    {
        _pool = [[NSAutoreleasePool alloc] init];
        
        _commandQueue = [commandQueue retain];
        _drawable = [drawable retain];
        _commandBuffer = [[commandQueue commandBuffer] retain];
        _renderPassDesc = [[MTLRenderPassDescriptor renderPassDescriptor] retain];
        
        MTLRenderPassColorAttachmentDescriptor *colorAttachment = _renderPassDesc.colorAttachments[0];
        
        // Clear to a red-orange color when beginning the render pass.
        colorAttachment.clearColor  = MTLClearColorMake(1.0, 0.3, 0.0, 1.0);
        colorAttachment.loadAction  = MTLLoadActionClear;
        colorAttachment.storeAction = MTLStoreActionStore;
        
        colorAttachment.texture = _drawable.texture;
        
        _encoder = [[_commandBuffer renderCommandEncoderWithDescriptor:_renderPassDesc] retain];
    }
    
    CommandEncoderMetal::~CommandEncoderMetal()
    {
        [_commandQueue release];
        [_commandBuffer release];
        [_renderPassDesc release];
        [_drawable release];
        [_encoder release];
        [_pool release];
    }
    
    void CommandEncoderMetal::setViewport(const glm::ivec4 &viewport)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::setShader(const std::shared_ptr<Shader> &abstractShader)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::setFragmentSampler(const std::shared_ptr<TextureSampler> &abstractSampler, size_t index)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::setFragmentBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::updateFence(const std::shared_ptr<Fence> &fence)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::waitForFence(const std::shared_ptr<Fence> &fence, std::function<void()> &&callback)
    {
        throw Exception("unimplemented");
    }
    
    void CommandEncoderMetal::onSubmit()
    {
        [_encoder endEncoding];
        [_commandBuffer presentDrawable:_drawable];
        [_commandBuffer commit];
    }
    
}; // namespace PinkTopaz::Renderer::Metal
