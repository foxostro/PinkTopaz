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
#include "Exception.hpp"
#include <vector>
#include <glm/gtc/type_ptr.hpp>

namespace PinkTopaz {
    
    Shader::Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        const GLchar *vert = (const GLchar *)vertexShaderSource.c_str();
        const GLchar *frag = (const GLchar *)fragmentShaderSource.c_str();

        std::vector<std::pair<GLenum, const GLchar *> > shaderType = {
            std::make_pair(GL_VERTEX_SHADER, vert),
            std::make_pair(GL_FRAGMENT_SHADER, frag),
        };
        
        _program = glCreateProgram();
        
        for(auto p : shaderType)
        {
            GLuint shader = glCreateShader(p.first);
            glShaderSource(shader, 1, &p.second, NULL);
            
            glCompileShader(shader);
            _checkShaderCompileStatus(shader);
            
            glAttachShader(_program, shader);
            glDeleteShader(shader); // We can delete the shader as soon as the program has a reference to it.
        }

        glLinkProgram(_program);
        _checkProgramLinkStatus();

        CHECK_GL_ERROR();
    }
    
    Shader::~Shader()
    {
        glDeleteProgram(_program);
        CHECK_GL_ERROR();
    }
    
    void Shader::use()
    {
        glUseProgram(_program);
        CHECK_GL_ERROR();
    }
    
    void Shader::setUniform(const GLchar *name, const glm::mat4 &value)
    {
        GLint loc = glGetUniformLocation(_program, name);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        CHECK_GL_ERROR();
    }
    
    void Shader::setUniform(const GLchar *name, GLint value)
    {
        GLint loc = glGetUniformLocation(_program, name);
        glUniform1i(loc, value);
        CHECK_GL_ERROR();
    }
    
    void Shader::_checkShaderCompileStatus(GLuint shader)
    {
        CHECK_GL_ERROR();
        
        GLint isCompiled = 0;
        
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        
        if (isCompiled == GL_FALSE) {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            
            std::vector<GLchar> infoLog(length);
            glGetShaderInfoLog(shader, length, &length, &infoLog[0]);
            throw Exception("Failed to compile shader: %s\n", infoLog.data());
        }
    }
    
    void Shader::_checkProgramLinkStatus()
    {
        CHECK_GL_ERROR();

        GLint isLinked = 0;
        
        glGetProgramiv(_program, GL_LINK_STATUS, &isLinked);
        
        if (isLinked == GL_FALSE) {
            GLint length = 0;
            glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);
            
            std::vector<GLchar> infoLog(length);
            glGetProgramInfoLog(_program, length, &length, &infoLog[0]);
            throw Exception("Failed to link shader program: %s\n", infoLog.data());
        }
    }
    
} // namespace PinkTopaz
