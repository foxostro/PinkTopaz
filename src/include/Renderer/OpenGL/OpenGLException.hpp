//
//  OpenGLException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef OpenGLException_hpp
#define OpenGLException_hpp

#include "Renderer/RendererException.hpp"

// Exception thrown when an OpenGL-specific error occurs in the OpenGL renderer.
class OpenGLException : public RendererException
{
public:
    template<typename... Args>
    OpenGLException(Args&&... args)
    : RendererException(std::forward<Args>(args)...)
    {}
};

#endif /* OpenGLException_hpp */
