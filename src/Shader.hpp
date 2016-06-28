//
//  Shader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef Shader_hpp
#define Shader_hpp

#include <OpenGL/gl3.h>
#include <string>

namespace PinkTopaz {
    
class Shader
{
public:
    Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);
    ~Shader();
    void use();
    
private:
    void checkShaderCompileStatus(GLuint shader);
    void checkProgramLinkStatus();
    
    GLuint program;
};
    
} // namespace PinkTopaz

#endif /* Shader_hpp */
