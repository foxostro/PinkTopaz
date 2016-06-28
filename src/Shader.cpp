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
    
    Shader::Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        const GLchar *vert = (const GLchar *)vertexShaderSource.c_str();
        const GLchar *frag = (const GLchar *)fragmentShaderSource.c_str();

        std::vector<std::pair<GLenum, const GLchar *> > shaderType = {
            std::make_pair(GL_VERTEX_SHADER, vert),
            std::make_pair(GL_FRAGMENT_SHADER, frag),
        };
        
        program = glCreateProgram();
        
        for(auto p : shaderType)
        {
            GLuint shader = glCreateShader(p.first);
            glShaderSource(shader, 1, &p.second, NULL);
            
            glCompileShader(shader);
            checkShaderCompileStatus(shader);
            
            glAttachShader(program, shader);
            glDeleteShader(shader); // We can delete the shader as soon as the program has a reference to it.
        }

        glLinkProgram(program);
        checkProgramLinkStatus();

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
