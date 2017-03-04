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
#include "Renderer/OpenGL/opengl.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

#include <map>

namespace PinkTopaz::Renderer::OpenGL {
    
    class ShaderOpenGL : public Shader
    {
    public:
        ShaderOpenGL(CommandQueue &commandQueue,
                     const std::string &vertexShaderSource,
                     const std::string &fragmentShaderSource);

        ~ShaderOpenGL();
        
        inline GLuint getProgram() const { return _program; }
        
    private:
        std::atomic<GLuint> _program;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* ShaderOpenGL_hpp */
