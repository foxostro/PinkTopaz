//
//  MetalException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef MetalException_hpp
#define MetalException_hpp

#include "Renderer/RendererException.hpp"

// Exception thrown when a Metal-specific error occurs in the Metal renderer.
class MetalException : public RendererException
{
public:
    template<typename... Args>
    MetalException(Args&&... args)
    : RendererException(std::forward<Args>(args)...)
    {}
};

#endif /* MetalException_hpp */
