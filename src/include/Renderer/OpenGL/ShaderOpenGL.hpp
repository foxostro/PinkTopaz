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

namespace PinkTopaz::Renderer::OpenGL {
    
    class ShaderOpenGL : public Shader
    {
    public:
        ShaderOpenGL(const VertexFormat &vertexFormat,
                     const std::string &vertexShaderSource,
                     const std::string &fragmentShaderSource,
                     bool blending);

        ~ShaderOpenGL();
        
        inline GLuint getProgram() const { return _program; }
        inline const VertexFormat& getVertexFormat() const { return _vertexFormat; }
        inline bool getBlending() const { return _blending; }
        
    private:
        GLuint _program;
        VertexFormat _vertexFormat;
        bool _blending;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* ShaderOpenGL_hpp */
