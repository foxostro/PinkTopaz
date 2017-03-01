//
//  RenderPassDescriptor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef RenderPassDescriptor_hpp
#define RenderPassDescriptor_hpp

namespace PinkTopaz::Renderer {
    
    struct RenderPassDescriptor
    {
        bool blend;
        bool depthTest;
        bool clear;
        
        RenderPassDescriptor()
         : blend(false),
           depthTest(true),
           clear(true)
        {}
    };
    
} // namespace PinkTopaz::Renderer

#endif /* RenderPassDescriptor_hpp */
