//
//  glUtilities.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef glUtilities_hpp
#define glUtilities_hpp

#include <OpenGL/gl3.h>

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
