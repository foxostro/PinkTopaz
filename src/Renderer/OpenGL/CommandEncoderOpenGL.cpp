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
#include "SDL.h"

namespace PinkTopaz::Renderer::OpenGL {
    
    CommandEncoderOpenGL::CommandEncoderOpenGL(const RenderPassDescriptor &desc)
    {
        _vertexFormat = desc.vertexFormat;
        
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
    }
    
    void CommandEncoderOpenGL::setViewport(const glm::ivec4 &viewport)
    {
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        CHECK_GL_ERROR();
    }
    
    void CommandEncoderOpenGL::setShader(const std::shared_ptr<Shader> &abstractShader)
    {
        assert(abstractShader);
        auto shader = std::dynamic_pointer_cast<ShaderOpenGL>(abstractShader);
        GLuint program = shader->getProgram();
        glUseProgram(program);
        CHECK_GL_ERROR();
    }
    
    void CommandEncoderOpenGL::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
    {
        assert(abstractTexture);
        auto texture = std::dynamic_pointer_cast<TextureOpenGL>(abstractTexture);
        GLuint handle = texture->getHandle();
        glActiveTexture(GL_TEXTURE0 + (GLenum)index);
        glBindTexture(texture->getTarget(), handle);
        CHECK_GL_ERROR();
    }
    
    void CommandEncoderOpenGL::setFragmentSampler(const std::shared_ptr<TextureSampler> &abstractSampler, size_t index)
    {
        assert(abstractSampler);
        auto sampler = std::dynamic_pointer_cast<TextureSamplerOpenGL>(abstractSampler);
        GLuint handle = sampler->getHandle();
        glBindSampler(index, handle);
        CHECK_GL_ERROR();
    }
    
    void CommandEncoderOpenGL::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {
        assert(abstractBuffer);
        auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);
        const GLuint vbo = buffer->getHandleVBO();
        const GLenum target = buffer->getTargetEnum();
        
        if (UniformBuffer == buffer->getType()) {
            glBindBufferBase(target, index, vbo);
        } else {
            GLuint vao = buffer->getHandleVAO();
            glBindVertexArray(vao);
            glBindBuffer(target, vbo);
            setupVertexAttributes(_vertexFormat);
            glBindBuffer(target, 0);
        }
        CHECK_GL_ERROR();
    }
    
    void CommandEncoderOpenGL::setFragmentBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
    {
        // OpenGL does not reserve separate namespaces for fragment buffer
        // indices and vertex buffer indices?
        setVertexBuffer(abstractBuffer, index);
    }
    
    void CommandEncoderOpenGL::drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances)
    {
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
    }
    
    void CommandEncoderOpenGL::updateFence(const std::shared_ptr<Fence> &fence) {}

    void CommandEncoderOpenGL::waitForFence(const std::shared_ptr<Fence> &f,
                                            std::function<void()> &&callback)
    {
        auto fence = std::dynamic_pointer_cast<FenceOpenGL>(f);
        GLsync object = fence->getObject();
        constexpr GLuint64 timeout = ~0;
        glClientWaitSync(object, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
        CHECK_GL_ERROR();
        callback();
    }
    
    void CommandEncoderOpenGL::setupVertexAttributes(const VertexFormat &format)
    {
        // Buffers are already bound coming into this method.
        
        for (size_t i = 0, n = format.attributes.size(); i < n; ++i)
        {
            const AttributeFormat &attr = format.attributes[i];
            
            GLenum typeEnum;
            switch (attr.type)
            {
                case AttributeTypeFloat:
                    typeEnum = GL_FLOAT;
                    break;
                    
                case AttributeTypeUnsignedByte:
                    typeEnum = GL_UNSIGNED_BYTE;
                    break;
                    
                default:
                    throw Exception("Unsupported buffer attribute type %d in BufferOpenGL\n", (int)attr.type);
            }
            
            glVertexAttribPointer((GLuint)i,
                                  (GLint)attr.size,
                                  typeEnum,
                                  attr.normalized ? GL_TRUE : GL_FALSE,
                                  (GLsizei)attr.stride,
                                  (const GLvoid *)attr.offset);
            glEnableVertexAttribArray((GLuint)i);
        }
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
