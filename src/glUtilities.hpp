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
#include <exception>
#include <string>

namespace PinkTopaz {
    
    class OpenGLException : public std::exception
    {
    private:
        std::string _reason;

    public:
        OpenGLException(const std::string fmt, ...);

        virtual const char *what() const noexcept override
        {
            return _reason.c_str();
        }
    };
    
    const char * stringForOpenGLError(GLenum error);
    void checkGLError();
    
} // namespace PinkTopaz

#endif /* glUtilities_hpp */
