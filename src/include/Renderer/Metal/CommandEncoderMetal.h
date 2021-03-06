//
//  CommandEncoderMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef CommandEncoderMetal_h
#define CommandEncoderMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#import "Renderer/CommandEncoder.hpp"
#import "Renderer/RenderPassDescriptor.hpp"
#import "Renderer/Metal/ShaderMetal.h"

#import <memory>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>

class CommandEncoderMetal : public CommandEncoder
{
public:
    CommandEncoderMetal(const RenderPassDescriptor &desc,
                        id <MTLDevice> device,
                        id <MTLCommandQueue> commandQueue,
                        id <CAMetalDrawable> drawable,
                        id <MTLTexture> depthTexture,
                        id <MTLDepthStencilState> depthTestOn,
                        id <MTLDepthStencilState> depthTestOff);
    
    virtual ~CommandEncoderMetal();
    
    void setViewport(const glm::ivec4 &viewport) override;
    void setShader(const std::shared_ptr<Shader> &shader) override;
    void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) override;
    void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) override;
    void setVertexBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
    void setFragmentBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
    void drawPrimitives(PrimitiveType primitiveType, size_t first, size_t count, size_t numInstances) override;
    void drawIndexedPrimitives(PrimitiveType primitiveType, size_t indexCount, const std::shared_ptr<Buffer> &indexBuffer, size_t instanceCount) override;
    void setTriangleFillMode(TriangleFillMode fillMode) override;
    void commit() override;
    void setDepthTest(bool enable) override;
    
private:
    id <MTLCommandQueue> _commandQueue;
    id <MTLCommandBuffer> _commandBuffer;
    MTLRenderPassDescriptor *_metalRenderPassDesc;
    id <CAMetalDrawable> _drawable;
    id <MTLRenderCommandEncoder> _encoder;
    id <MTLDepthStencilState> _depthTestOn;
    id <MTLDepthStencilState> _depthTestOff;
};

#endif /* CommandEncoderMetal_h */
