//
//  ShaderOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef ShaderOpenGL_hpp
#define ShaderOpenGL_hpp

#include "Renderer/Shader.hpp"
#include "Renderer/VertexFormat.hpp"
#include "Renderer/OpenGL/opengl.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/OpenGLException.hpp"


// Exception for when a GLSL shader fails to compile.
class ShaderCompileErrorOpenGLException : public OpenGLException
{
public:
    ShaderCompileErrorOpenGLException(GLuint shader, const std::vector<GLchar> &infoLog)
    : OpenGLException("Failed to compile shader {}: {}", shader, infoLog.data())
    {}
};


// Exception for when a GLSL program fails to link.
class ProgramLinkErrorOpenGLException : public OpenGLException
{
public:
    ProgramLinkErrorOpenGLException(GLuint program, const std::vector<GLchar> &infoLog)
    : OpenGLException("Failed to link shader program {}: {}", program, infoLog.data())
    {}
};


class ShaderOpenGL : public Shader
{
public:
    ShaderOpenGL(unsigned id,
                 const std::shared_ptr<CommandQueue> &commandQueue,
                 const VertexFormat &vertexFormat,
                 const std::string &vertexShaderSource,
                 const std::string &fragmentShaderSource,
                 bool blending);
    
    ~ShaderOpenGL();
    
    inline GLuint getProgram() const { return _program; }
    inline const VertexFormat& getVertexFormat() const { return _vertexFormat; }
    inline bool getBlending() const { return _blending; }
    
private:
    unsigned _id;
    GLuint _program;
    VertexFormat _vertexFormat;
    bool _blending;
    std::shared_ptr<CommandQueue> _commandQueue;
};

#endif /* ShaderOpenGL_hpp */
