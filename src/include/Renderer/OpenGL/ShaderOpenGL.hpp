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
