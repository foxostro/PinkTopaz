//
//  RendererException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef RendererException_hpp
#define RendererException_hpp

#include "Exception.hpp"

// Exception thrown when an error occurs in the OpenGL renderer.
class RendererException : public Exception
{
public:
    template<typename... Args>
    RendererException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

#endif /* RendererException_hpp */
