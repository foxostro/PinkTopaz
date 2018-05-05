//
//  CommandEncoderMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#import "Renderer/Metal/CommandEncoderMetal.h"
#import "Renderer/Metal/ShaderMetal.h"
#import "Renderer/Metal/BufferMetal.h"
#import "Renderer/Metal/TextureMetal.h"
#import "Renderer/Metal/TextureSamplerMetal.h"
#import "Exception.hpp"

CommandEncoderMetal::CommandEncoderMetal(const RenderPassDescriptor &desc,
                                         id <MTLDevice> device,
                                         id <MTLCommandQueue> commandQueue,
                                         id <CAMetalDrawable> drawable,
                                         id <MTLTexture> depthTexture,
                                         id <MTLDepthStencilState> depthTestOn,
                                         id <MTLDepthStencilState> depthTestOff)
{
    _pool = [[NSAutoreleasePool alloc] init];
    _commandQueue = [commandQueue retain];
    _drawable = [drawable retain];
    _commandBuffer = [[commandQueue commandBuffer] retain];
    _metalRenderPassDesc = [[MTLRenderPassDescriptor renderPassDescriptor] retain];
    
    MTLRenderPassColorAttachmentDescriptor *colorAttachment = _metalRenderPassDesc.colorAttachments[0];
    colorAttachment.texture = _drawable.texture;
    colorAttachment.clearColor  = MTLClearColorMake(desc.clearColor.r,
                                                    desc.clearColor.g,
                                                    desc.clearColor.b,
                                                    desc.clearColor.a);
    colorAttachment.storeAction = MTLStoreActionStore;
    colorAttachment.loadAction  = desc.clear ? MTLLoadActionClear : MTLLoadActionLoad;
    
    MTLRenderPassDepthAttachmentDescriptor *depthAttachment = _metalRenderPassDesc.depthAttachment;
    depthAttachment.texture = depthTexture;
    depthAttachment.clearDepth = 1.0;
    depthAttachment.storeAction = MTLStoreActionDontCare;
    depthAttachment.loadAction = desc.clear ? MTLLoadActionClear : MTLLoadActionLoad;
    
    _encoder = [[_commandBuffer renderCommandEncoderWithDescriptor:_metalRenderPassDesc] retain];
    
    [_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [_encoder setCullMode:MTLCullModeBack];
    [_encoder setDepthStencilState:depthTestOn];
    
    _depthTestOn = [depthTestOn retain];
    _depthTestOff = [depthTestOff retain];
}

CommandEncoderMetal::~CommandEncoderMetal()
{
    [_commandQueue release];
    [_commandBuffer release];
    [_metalRenderPassDesc release];
    [_drawable release];
    [_encoder release];
    [_depthTestOn release];
    [_depthTestOff release];
}

void CommandEncoderMetal::setViewport(const glm::ivec4 &viewport)
{
    [_encoder setViewport:(MTLViewport) {
        .originX    = (double)viewport.x,
        .originY    = (double)viewport.y,
        .width      = (double)viewport.z,
        .height     = (double)viewport.w,
        .znear      = 0.0,
        .zfar       = 1.0
    }];
}

void CommandEncoderMetal::setShader(const std::shared_ptr<Shader> &abstractShader)
{
    auto shader = std::dynamic_pointer_cast<ShaderMetal>(abstractShader);
    id <MTLRenderPipelineState> pipelineState = shader->getPipelineState();
    [_encoder setRenderPipelineState:pipelineState];
}

void CommandEncoderMetal::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
{
    auto concreteTexture = std::dynamic_pointer_cast<TextureMetal>(abstractTexture);
    id <MTLTexture> metalTexture = concreteTexture->getMetalTexture();
    [_encoder setFragmentTexture:metalTexture atIndex:index];
}

void CommandEncoderMetal::setFragmentSampler(const std::shared_ptr<TextureSampler> &abstractSampler, size_t index)
{
    auto concreteSampler = std::dynamic_pointer_cast<TextureSamplerMetal>(abstractSampler);
    id <MTLSamplerState> metalSampler = concreteSampler->getMetalSampler();
    [_encoder setFragmentSamplerState:metalSampler atIndex:index];
}

void CommandEncoderMetal::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
{
    auto concreteBuffer = std::dynamic_pointer_cast<BufferMetal>(abstractBuffer);
    id <MTLBuffer> metalBuffer = concreteBuffer->getMetalBuffer();
    [_encoder setVertexBuffer:metalBuffer offset:0 atIndex:index];
}

void CommandEncoderMetal::setFragmentBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
{
    auto concreteBuffer = std::dynamic_pointer_cast<BufferMetal>(abstractBuffer);
    id <MTLBuffer> metalBuffer = concreteBuffer->getMetalBuffer();
    [_encoder setFragmentBuffer:metalBuffer
                         offset:0
                        atIndex:index];
}

void CommandEncoderMetal::drawPrimitives(PrimitiveType primitiveType,
                                         size_t first, size_t count,
                                         size_t numInstances)
{
    MTLPrimitiveType metalPrimitiveType;
    
    switch (primitiveType) {
        case Triangles:
            metalPrimitiveType = MTLPrimitiveTypeTriangle;
            break;
        
        case TriangleStrip:
            metalPrimitiveType = MTLPrimitiveTypeTriangleStrip;
            break;
            
        default:
            throw Exception("Unsupported primitive type in drawPrimitives().");
    }
    
    [_encoder drawPrimitives:metalPrimitiveType
                 vertexStart:first
                 vertexCount:count
               instanceCount:numInstances
                baseInstance:0];
}

void CommandEncoderMetal::setTriangleFillMode(TriangleFillMode fillMode)
{
    MTLTriangleFillMode metalFillMode;
    
    switch (fillMode)
    {
        case Fill:
            metalFillMode = MTLTriangleFillModeFill;
            break;
            
        case Lines:
            metalFillMode = MTLTriangleFillModeLines;
            break;
            
        default:
            throw Exception("Unsupported triangle fill mode in setTriangleFillMode().");
    }
    
    [_encoder setTriangleFillMode:metalFillMode];
}

void CommandEncoderMetal::commit()
{
    [_encoder endEncoding];
    [_commandBuffer presentDrawable:_drawable];
    [_commandBuffer commit];
    
    [_pool release];
    _pool = nil;
}

void CommandEncoderMetal::setDepthTest(bool enable)
{
    [_encoder setDepthStencilState:(enable ? _depthTestOn : _depthTestOff)];
}
