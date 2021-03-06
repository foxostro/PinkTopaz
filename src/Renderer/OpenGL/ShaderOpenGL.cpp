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
#include <vector>
#include "SDL.h"

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
        throw ShaderCompileErrorOpenGLException(shader, infoLog);
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
        throw ProgramLinkErrorOpenGLException(program, infoLog);
    }
}

ShaderOpenGL::ShaderOpenGL(unsigned id,
                           const std::shared_ptr<CommandQueue> &commandQueue,
                           const VertexFormat &vertexFormat,
                           const std::string &vertexShaderSource,
                           const std::string &fragmentShaderSource,
                           bool blending)
 : _id(id),
   _program(0),
   _vertexFormat(vertexFormat),
   _blending(blending),
   _commandQueue(commandQueue)
{
    _commandQueue->enqueue(_id, __FUNCTION__, [=]{
        const GLchar *vert = (const GLchar *)vertexShaderSource.c_str();
        const GLchar *frag = (const GLchar *)fragmentShaderSource.c_str();
        
        std::vector<std::pair<GLenum, const GLchar *> > shaderType = {
            std::make_pair(GL_VERTEX_SHADER, vert),
            std::make_pair(GL_FRAGMENT_SHADER, frag),
        };
        
        GLuint program = glCreateProgram();
        _program = program;
        
        for (auto p : shaderType) {
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
    const GLuint program = _program;
    _commandQueue->cancel(_id);
    _commandQueue->enqueue(0, __FUNCTION__, [program]{
        if (program) {
            glDeleteProgram(program);
            CHECK_GL_ERROR();
        }
    });
}
