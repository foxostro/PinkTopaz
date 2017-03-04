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
        
        void setShaderUniform(const char *name, const glm::mat4 &value) override;
        void setShaderUniform(const char *name, const glm::vec3 &value) override;
        void setShaderUniform(const char *name, int value) override;
        
        inline GLuint getProgram() const { return _program; }
        
    private:
        GLint getUniformLocation(GLuint program, const char *name);
        
        std::atomic<GLuint> _program;
        CommandQueue &_commandQueue;
        std::map<const char *, GLint> _loc;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* ShaderOpenGL_hpp */
