//
//  CommandEncoderMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#import "Renderer/Metal/CommandEncoderMetal.h"
#import "Renderer/Metal/ShaderMetal.h"
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
        colorAttachment.texture = _drawable.texture;
        colorAttachment.storeAction = MTLStoreActionStore;
        
        if (desc.clear) {
            colorAttachment.clearColor  = MTLClearColorMake(0.2, 0.4, 0.5, 1.0);
            colorAttachment.loadAction  = MTLLoadActionClear;
        }
        
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
        [_encoder setViewport:(MTLViewport) {
            .originX    = (double)viewport.x,
            .originY    = (double)viewport.y,
            .width      = (double)viewport.z,
            .height     = (double)viewport.w,
            .znear      = -0.1,
            .zfar       = 100.0
        }];
    }
    
    void CommandEncoderMetal::setShader(const std::shared_ptr<Shader> &abstractShader)
    {
        auto shader = std::dynamic_pointer_cast<ShaderMetal>(abstractShader);
        id <MTLRenderPipelineState> pipelineState = shader->getPipelineState();
        [_encoder setRenderPipelineState:pipelineState];
    }
    
    void CommandEncoderMetal::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
    {}
    
    void CommandEncoderMetal::setFragmentSampler(const std::shared_ptr<TextureSampler> &abstractSampler, size_t index)
    {}
    
    void CommandEncoderMetal::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {}
    
    void CommandEncoderMetal::setFragmentBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {}
    
    void CommandEncoderMetal::drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances)
    {}
    
    void CommandEncoderMetal::updateFence(const std::shared_ptr<Fence> &fence)
    {}
    
    void CommandEncoderMetal::waitForFence(const std::shared_ptr<Fence> &fence, std::function<void()> &&callback)
    {}
    
    void CommandEncoderMetal::onSubmit()
    {
        [_encoder endEncoding];
        [_commandBuffer presentDrawable:_drawable];
        [_commandBuffer commit];
    }
    
} // namespace PinkTopaz::Renderer::Metal
