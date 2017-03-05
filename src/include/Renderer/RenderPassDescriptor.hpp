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

namespace PinkTopaz::Renderer {
    
    struct RenderPassDescriptor
    {
        bool depthTest;
        bool clear;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* RenderPassDescriptor_hpp */
