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
#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/TextureSamplerOpenGL.hpp"
#include "Renderer/OpenGL/BufferOpenGL.hpp"
#include "Renderer/OpenGL/FenceOpenGL.hpp"
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
    
    void CommandEncoderOpenGL::setFragmentSampler(const std::shared_ptr<TextureSampler> &abstractSampler, size_t index)
    {
        assert(abstractSampler);
        _commandQueue.enqueue([=]() {
            auto sampler = std::dynamic_pointer_cast<TextureSamplerOpenGL>(abstractSampler);
            GLuint handle = sampler->getHandle();
            glBindSampler(index, handle);
            CHECK_GL_ERROR();
        });
    }
    
    void CommandEncoderOpenGL::setFragmentBuffer(const std::shared_ptr<Buffer> &buffer, size_t index)
    {
        // OpenGL doesn't make a distinction between slots for fragment programs
        // and slots for vertex programs?
        setVertexBuffer(buffer, index);
    }
    
    void CommandEncoderOpenGL::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {
        assert(abstractBuffer);
        _commandQueue.enqueue([=]() {
            auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);
            
            GLuint vao = buffer->getHandleVAO();
            GLuint vbo = buffer->getHandleVBO();
            GLenum target = buffer->getTargetEnum();
            
            glBindVertexArray(vao);
            glBindBufferBase(target, index, vbo);
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
    
    void CommandEncoderOpenGL::updateFence(const std::shared_ptr<Fence> &fence)
    {
        
    }

    void CommandEncoderOpenGL::waitForFence(const std::shared_ptr<Fence> &f,
                                            std::function<void()> &&callback)
    {
        _commandQueue.enqueue([=, callback{std::move(callback)}]() {
            auto fence = std::dynamic_pointer_cast<FenceOpenGL>(f);
            GLsync object = fence->getObject();
            constexpr GLuint64 timeout = ~0;
            glClientWaitSync(object, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
            CHECK_GL_ERROR();
            callback();
        });
    }
    
}; // namespace PinkTopaz::Renderer::OpenGL
