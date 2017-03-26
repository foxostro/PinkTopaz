//
//  RenderPassDescriptor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef RenderPassDescriptor_hpp
#define RenderPassDescriptor_hpp

#include "Renderer/VertexFormat.hpp"

namespace Renderer {
    
    struct RenderPassDescriptor
    {
        // Set to `true' in order to clear Color and Depth and Stencil when the
        // rendering pass begins.
        bool clear;
        
        // The color to clear the render target to should `clear' be true.
        glm::vec4 clearColor;
    };
    
} // namespace Renderer

#endif /* RenderPassDescriptor_hpp */
