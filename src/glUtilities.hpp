//
//  glUtilities.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef glUtilities_hpp
#define glUtilities_hpp

#include "opengl.hpp"

#ifdef NDEBUG
#define CHECK_GL_ERROR(...)
#else
#define CHECK_GL_ERROR(...) checkGLError(__VA_ARGS__)
#endif

namespace PinkTopaz {
    
    const char * stringForOpenGLError(GLenum error);
    void checkGLError();
    
} // namespace PinkTopaz

#endif /* glUtilities_hpp */
