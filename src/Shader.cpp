//
//  Shader.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#include "SDL.h"
#include "Shader.hpp"
#include "glUtilities.hpp"
#include <vector>

namespace PinkTopaz {
    
    Shader::Shader(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource) : program(0)
    {
        GLuint vertexShader = 0;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        
        GLuint fragmentShader = 0;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        
        glCompileShader(vertexShader);
        checkShaderCompileStatus(vertexShader);
        
        glCompileShader(fragmentShader);
        checkShaderCompileStatus(fragmentShader);
        
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        checkProgramLinkStatus();
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        checkGLError();
    }
    
    Shader::~Shader()
    {
        glDeleteProgram(program);
        checkGLError();
    }
    
    void Shader::use()
    {
        glUseProgram(program);
        checkGLError();
    }
    
    void Shader::checkShaderCompileStatus(GLuint shader)
    {
        checkGLError();
        
        GLint isCompiled = 0;
        
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        
        if (isCompiled == GL_FALSE) {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            
            std::vector<GLchar> infoLog(length);
            glGetShaderInfoLog(shader, length, &length, &infoLog[0]);
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to compile shader: %s\n", infoLog.data());
            abort();
        }
    }
    
    void Shader::checkProgramLinkStatus()
    {
        checkGLError();

        GLint isLinked = 0;
        
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        
        if (isLinked == GL_FALSE) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            
            std::vector<GLchar> infoLog(length);
            glGetProgramInfoLog(program, length, &length, &infoLog[0]);
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to link shader program: %s\n", infoLog.data());
            abort();
        }
    }
    
} // namespace PinkTopaz
