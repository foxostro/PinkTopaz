//
//  opengl.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#ifndef opengl_h
#define opengl_h

// The OpenGL headers are located at a different place, and have different names, on different
// platforms. This header is intended to be a central place where this differentiation can be done
// for PinkTopaz.
#ifdef __APPLE__
#define GL3_PROTOTYPES
#include <OpenGL/gl3.h>
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   define GL_GLEXT_PROTOTYPES
#   include <GL/gl.h>
#endif

#include <glew.h>

#endif /* opengl_h */
