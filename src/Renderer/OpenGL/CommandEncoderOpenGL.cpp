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
#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/BufferOpenGL.hpp"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    CommandEncoderOpenGL::CommandEncoderOpenGL(const RenderPassDescriptor &desc)
    {
        _commandQueue.enqueue([desc]() {
            if (desc.blend) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else {
                glDisable(GL_BLEND);
            }
            
            if (desc.depthTest) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
            } else {
                glDisable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
            }
            
            if (desc.clear) {
                // According to <https://www.khronos.org/opengl/wiki/Common_Mistakes#Swap_Buffers>
                // it is important to clear all three buffers for best performance.
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }

            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setViewport(const glm::ivec4 &viewport)
    {
        _commandQueue.enqueue([=]() {
            glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setShader(const std::shared_ptr<Shader> &abstractShader)
    {
        assert(abstractShader);
        _commandQueue.enqueue([=]() {
            auto shader = std::dynamic_pointer_cast<ShaderOpenGL>(abstractShader);
            GLuint program = shader->getProgram();
            glUseProgram(program);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setFragmentTexture(const std::shared_ptr<TextureArray> &abstractTexture, size_t index)
    {
        assert(abstractTexture);
        _commandQueue.enqueue([=]() {
            auto texture = std::dynamic_pointer_cast<TextureArrayOpenGL>(abstractTexture);
            GLuint handle = texture->getHandle();
            glActiveTexture(GL_TEXTURE0 + (GLenum)index);
            glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
    {
        assert(abstractTexture);
        _commandQueue.enqueue([=]() {
            auto texture = std::dynamic_pointer_cast<TextureOpenGL>(abstractTexture);
            GLuint handle = texture->getHandle();
            glActiveTexture(GL_TEXTURE0 + (GLenum)index);
            glBindTexture(texture->getTarget(), handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer)
    {
        assert(abstractBuffer);
        _commandQueue.enqueue([=]() {
            auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);
            GLuint handle = buffer->getHandleVAO();
            glBindVertexArray(handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setVertexBytes(const std::shared_ptr<Buffer> &abstractBuffer, size_t size, const void *data)
    {
        std::vector<uint8_t> wrappedData(size);
        memcpy(&wrappedData[0], data, size);
        
        _commandQueue.enqueue([=]() {
            auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);

            size_t n = wrappedData.size();
            const void *p = (const void *)&wrappedData[0];
            
            GLuint vao = buffer->getHandleVAO();
            GLuint vbo = buffer->getHandleVBO();
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, n, p);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
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
            
            glDrawArrays(mode, (GLint)first, (GLsizei)count);
            CHECK_GL_ERROR();
        });
    }
    
}; // namespace PinkTopaz::Renderer::OpenGL
