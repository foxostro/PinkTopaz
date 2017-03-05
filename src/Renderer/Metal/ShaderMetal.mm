//
//  ShaderMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/ShaderMetal.h"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {
    
    static MTLVertexFormat getVertexFormatEnum(const AttributeFormat &attr)
    {
        if (AttributeTypeFloat == attr.type && 3 == attr.size) {
            return MTLVertexFormatFloat3;
        }
        
        if (AttributeTypeFloat == attr.type && 4 == attr.size) {
            return MTLVertexFormatFloat4;
        }
        
        if (AttributeTypeUnsignedByte == attr.type && 4 == attr.size) {
            return MTLVertexFormatUChar4;
        }
        
        throw Exception("Unsupported buffer attribute type\n");
    }
    
    static MTLVertexDescriptor*
    getVertexDescriptor(const VertexFormat &format)
    {
        MTLVertexDescriptor *desc = [MTLVertexDescriptor vertexDescriptor];
        size_t stride = 0;
        
        for (size_t i = 0, n = format.attributes.size(); i < n; ++i)
        {
            const AttributeFormat &attr = format.attributes[i];
            
            MTLVertexAttributeDescriptor *metalAttr = desc.attributes[i];
            metalAttr.format = getVertexFormatEnum(attr);
            metalAttr.offset = attr.offset;
            metalAttr.bufferIndex = 0;
            
            stride += attr.stride;
        }
        
        MTLVertexBufferLayoutDescriptor *layout = desc.layouts[0];
        layout.stride = stride;
        layout.stepFunction = MTLVertexStepFunctionPerVertex;
        layout.stepRate = 1;
        
        return desc;
    }

    ShaderMetal::ShaderMetal(const VertexFormat &vertexFormat,
                             id <MTLDevice> device,
                             id<MTLLibrary> library,
                             const std::string &vert,
                             const std::string &frag)
    {
        @autoreleasepool {
            NSError *error;
            
            MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
            desc.vertexFunction = [library newFunctionWithName:[NSString stringWithUTF8String:vert.c_str()]];
            desc.fragmentFunction = [library newFunctionWithName:[NSString stringWithUTF8String:frag.c_str()]];
            desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            desc.vertexDescriptor = getVertexDescriptor(vertexFormat);
            
            error = nil;
            _pipelineState = [device newRenderPipelineStateWithDescriptor:desc error:&error];
            if (!_pipelineState) {
                NSString *errorDesc = [error localizedDescription];
                throw Exception("Failed to create Metal pipeline state object: %s", errorDesc.UTF8String);
            }
        }
    }
    
    ShaderMetal::~ShaderMetal()
    {
        [_pipelineState release];
    }

} // namespace PinkTopaz::Renderer::Metal
