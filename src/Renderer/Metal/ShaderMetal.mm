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
