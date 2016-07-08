//
//  Shader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef Shader_hpp
#define Shader_hpp

#include "opengl.hpp"
#include <string>
#include <glm/mat4x4.hpp>

namespace PinkTopaz {
    
class Shader
{
public:
    Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);
    ~Shader();
    
    // Retrieve the OpenGL handle to the shader program.
    inline GLuint getProgram() const
    {
        return _program;
    }
    
    // Bind the shader program for use.
    void bind();
    void unbind();
    
    // Set a mat4 uniform by name. Assumes the shader has already been bound for use.
    void setUniform(const GLchar *name, const glm::mat4 &value);
    
    // Set an integer uniform by name. Assumes the shader has already been bound for use.
    void setUniform(const GLchar *name, GLint value);
    
private:
    void _checkShaderCompileStatus(GLuint shader);
    void _checkProgramLinkStatus();
    
    GLuint _program;
};
    
} // namespace PinkTopaz

#endif /* Shader_hpp */
