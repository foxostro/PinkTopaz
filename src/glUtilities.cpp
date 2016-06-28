//
//  glUtilities.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#include "glUtilities.hpp"
#include "SDL.h"

namespace PinkTopaz {
    
    const char * stringForOpenGLError(GLenum error)
    {
        switch(error)
        {
            case GL_NO_ERROR:
                return "GL_NO_ERROR. No error has been recorded.";
                
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM. An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
                
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE. A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
                
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION. The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
                
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION. The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
                
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY. There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
                
            default:
                return "Unknown OpenGL error";
        }
    }
    
    void checkGLError()
    {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            const char *errorStr = stringForOpenGLError(error);
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "glGetError: %s (%d)\n", errorStr, error);
            abort();
        }
    }
    
} // namespace PinkTopaz
