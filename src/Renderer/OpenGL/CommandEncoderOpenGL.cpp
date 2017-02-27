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
#include "Renderer/OpenGL/BufferOpenGL.hpp"
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
    
    void CommandEncoderOpenGL::setFragmentTexture(const std::shared_ptr<TextureArray> &abstractTexture, size_t index)
    {
        _commandQueue.enqueue([=]() {
            auto texture = std::dynamic_pointer_cast<TextureArrayOpenGL>(abstractTexture);
            GLuint handle = texture->getHandle();
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer)
    {
        _commandQueue.enqueue([=]() {
            auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);
            GLuint handle = buffer->getHandle();
            glBindVertexArray(handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances)
    {
        _commandQueue.enqueue([=]() {
            GLenum mode;
            switch (type) {
                case Triangles:
                    mode = GL_TRIANGLES;
                    break;

                default:
                    throw Exception("Invalid primitive type %d in call to drawPrimitives.\n", (int)type);
            }
            glDrawArrays(mode, first, count);
            CHECK_GL_ERROR();
        });
    }
    
}; // namespace PinkTopaz::Renderer::OpenGL
