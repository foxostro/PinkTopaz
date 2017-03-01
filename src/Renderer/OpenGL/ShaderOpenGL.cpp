//
//  ShaderOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#include "Renderer/OpenGL/ShaderOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace PinkTopaz::Renderer::OpenGL {
    
    static void checkShaderCompileStatus(GLuint shader)
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
    
    static void checkProgramLinkStatus(GLuint program)
    {
        CHECK_GL_ERROR();
        
        GLint isLinked = 0;
        
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        
        if (isLinked == GL_FALSE) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            
            std::vector<GLchar> infoLog(length);
            glGetProgramInfoLog(program, length, &length, &infoLog[0]);
            throw Exception("Failed to link shader program: %s\n", infoLog.data());
        }
    }

    ShaderOpenGL::ShaderOpenGL(const std::shared_ptr<CommandQueue> &commandQueue,
                               const std::string &vertexShaderSource,
                               const std::string &fragmentShaderSource)
    : _program(0), _commandQueue(commandQueue)
    {
        assert(_commandQueue);
        _commandQueue->enqueue([=] {
            const GLchar *vert = (const GLchar *)vertexShaderSource.c_str();
            const GLchar *frag = (const GLchar *)fragmentShaderSource.c_str();
            
            std::vector<std::pair<GLenum, const GLchar *> > shaderType = {
                std::make_pair(GL_VERTEX_SHADER, vert),
                std::make_pair(GL_FRAGMENT_SHADER, frag),
            };
            
            GLuint program = glCreateProgram();
            this->_program = program;
            
            for(auto p : shaderType)
            {
                GLuint shaderObject = glCreateShader(p.first);
                glShaderSource(shaderObject, 1, &p.second, NULL);
                
                glCompileShader(shaderObject);
                checkShaderCompileStatus(shaderObject);
                
                glAttachShader(program, shaderObject);
                glDeleteShader(shaderObject); // We can delete the shader as soon as the program has a reference to it.
            }
            
            glLinkProgram(program);
            checkProgramLinkStatus(program);
            
            CHECK_GL_ERROR();
        });
    }
    
    ShaderOpenGL::~ShaderOpenGL()
    {
        GLuint program = _program;
        _commandQueue->enqueue([program]{
            glDeleteProgram(program);
            CHECK_GL_ERROR();
        });
    }
    
    void ShaderOpenGL::setShaderUniform(const char *name, const glm::mat4 &value)
    {
        _commandQueue->enqueue([=]() {
            GLuint program = this->_program;
            GLint loc = glGetUniformLocation(program, name);
            glProgramUniformMatrix4fv(program, loc, 1, GL_FALSE, glm::value_ptr(value));
            CHECK_GL_ERROR();
        });
    }
    
    void ShaderOpenGL::setShaderUniform(const char *name, const glm::vec3 &value)
    {
        _commandQueue->enqueue([=]() {
            GLuint program = this->_program;
            GLint loc = glGetUniformLocation(program, name);
            glProgramUniform3f(program, loc, value.x, value.y, value.z);
            CHECK_GL_ERROR();
        });
    }
    
    void ShaderOpenGL::setShaderUniform(const char *name, int value)
    {
        _commandQueue->enqueue([=]() {
            GLuint program = this->_program;
            GLint loc = glGetUniformLocation(program, name);
            glProgramUniform1i(program, loc, value);
            CHECK_GL_ERROR();
        });
    }

} // namespace PinkTopaz::Renderer::OpenGL
