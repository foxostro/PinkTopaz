//
//  glUtilities.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#include "glUtilities.hpp"
#include "SDL.h"

#include <stdarg.h>
#include <memory>

namespace PinkTopaz {

    OpenGLException::OpenGLException(const std::string fmt_str, ...)
    {
        // http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
        int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
        std::string str;
        std::unique_ptr<char[]> formatted;
        va_list ap;
        while(1) {
            formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
            strcpy(&formatted[0], fmt_str.c_str());
            va_start(ap, fmt_str);
            final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
            va_end(ap);
            if (final_n < 0 || final_n >= n)
                n += abs(final_n - n + 1);
            else
                break;
        }
        
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", formatted.get());
    }
    
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
            throw OpenGLException("glGetError: %s (%d)\n", stringForOpenGLError(error), error);
        }
    }
    
} // namespace PinkTopaz
