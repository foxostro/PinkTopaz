//
//  CommandEncoderMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef CommandEncoderMetal_hpp
#define CommandEncoderMetal_hpp

#import "Renderer/CommandEncoder.hpp"
#import "Renderer/RenderPassDescriptor.hpp"

#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class CommandEncoderMetal : public CommandEncoder
    {
    public:
        CommandEncoderMetal(const RenderPassDescriptor &desc,
                            id <MTLCommandQueue> commandQueue,
                            id <CAMetalDrawable> drawable);
        
        virtual ~CommandEncoderMetal();
        
        void setViewport(const glm::ivec4 &viewport) override;
        void setShader(const std::shared_ptr<Shader> &shader) override;
        void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) override;
        void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) override;
        void setVertexBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
        void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) override;
        void updateFence(const std::shared_ptr<Fence> &fence) override;
        void waitForFence(const std::shared_ptr<Fence> &fence,
                          std::function<void()> &&completionHandler) override;
        
        // Called by the graphics device when the encoder is submitted.
        void onSubmit();
        
    private:
        id <MTLCommandQueue> _commandQueue;
        id <MTLCommandBuffer> _commandBuffer;
        MTLRenderPassDescriptor *_renderPassDesc;
        id <CAMetalDrawable> _drawable;
        id <MTLRenderCommandEncoder> _encoder;
        NSAutoreleasePool *_pool;
    };
    
}; // namespace PinkTopaz::Renderer::Metal

#endif /* CommandEncoderMetal_hpp */
