//
//  glUtilities.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef glUtilities_hpp
#define glUtilities_hpp

#include "Renderer/OpenGL/opengl.hpp"

#ifdef NDEBUG
#define CHECK_GL_ERROR(...)
#else
#define CHECK_GL_ERROR(...) checkGLError(__VA_ARGS__)
#endif

namespace Renderer {
    
    const char * stringForOpenGLError(GLenum error);
    void checkGLError();
    
} // namespace Renderer

#endif /* glUtilities_hpp */
