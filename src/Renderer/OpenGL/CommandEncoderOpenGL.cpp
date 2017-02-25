//
//  CommandEncoderOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#include "Renderer/OpenGL/CommandEncoderOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Renderer/OpenGL/ShaderOpenGL.hpp"
#include "Renderer/OpenGL/TextureArrayOpenGL.hpp"
#include "Renderer/OpenGL/StaticMeshVaoOpenGL.hpp"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    void CommandEncoderOpenGL::setViewport(const glm::ivec4 &viewport)
    {
        _commandQueue.enqueue([=]() {
            glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setShader(const std::shared_ptr<Shader> &abstractShader)
    {
        _commandQueue.enqueue([=]() {
            auto shader = std::dynamic_pointer_cast<ShaderOpenGL>(abstractShader);
            GLuint program = shader->getProgram();
            glUseProgram(program);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setTexture(const std::shared_ptr<TextureArray> &abstractTexture)
    {
        _commandQueue.enqueue([=]() {
            auto texture = std::dynamic_pointer_cast<TextureArrayOpenGL>(abstractTexture);
            GLuint handle = texture->getHandle();
            glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setVertexArray(const std::shared_ptr<StaticMeshVao> &abstractVao)
    {
        _commandQueue.enqueue([=]() {
            auto vao = std::dynamic_pointer_cast<StaticMeshVaoOpenGL>(abstractVao);
            GLuint handle = vao->getHandle();
            glBindVertexArray(handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::drawTriangles(size_t first, size_t count)
    {
        _commandQueue.enqueue([=]() {
            glDrawArrays(GL_TRIANGLES, first, count);
            CHECK_GL_ERROR();
        });
    }
    
}; // namespace PinkTopaz::Renderer::OpenGL
