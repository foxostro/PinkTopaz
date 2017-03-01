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

namespace PinkTopaz::Renderer::OpenGL {
    
    class ShaderOpenGL : public Shader
    {
    public:
        ShaderOpenGL(const std::shared_ptr<CommandQueue> &commandQueue,
                     const std::string &vertexShaderSource,
                     const std::string &fragmentShaderSource);

        ~ShaderOpenGL();
        
        void setShaderUniform(const char *name, const glm::mat4 &value) override;
        void setShaderUniform(const char *name, const glm::vec3 &value) override;
        void setShaderUniform(const char *name, int value) override;
        
        inline GLuint getProgram() const { return _program; }
        
    private:
        GLuint _program;
        const std::shared_ptr<CommandQueue> &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* ShaderOpenGL_hpp */
