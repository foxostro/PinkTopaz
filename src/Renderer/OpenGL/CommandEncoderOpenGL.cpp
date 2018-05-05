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
#include "Exception.hpp"
#include "SDL.h"

CommandEncoderOpenGL::CommandEncoderOpenGL(unsigned id,
                                           const std::shared_ptr<CommandQueue> &commandQueue,
                                           const RenderPassDescriptor &desc)
 : _id(id),
   _mainCommandQueue(commandQueue)
{
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        if (desc.clear) {
            glClearColor(desc.clearColor.r, desc.clearColor.g,
                         desc.clearColor.b, desc.clearColor.a);
            
            // According to <https://www.khronos.org/opengl/wiki/Common_Mistakes#Swap_Buffers>
            // it is important to clear all three buffers for best performance.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        
        internalSetDepthTest(true);
        
        CHECK_GL_ERROR();
    });
}

void CommandEncoderOpenGL::setViewport(const glm::ivec4 &viewport)
{
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        CHECK_GL_ERROR();
    });
}

void CommandEncoderOpenGL::setShader(const std::shared_ptr<Shader> &abstractShader)
{
    assert(abstractShader);
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        auto shader = std::dynamic_pointer_cast<ShaderOpenGL>(abstractShader);
        GLuint program = shader->getProgram();
        glUseProgram(program);
        
        if (shader->getBlending()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
        
        CHECK_GL_ERROR();
        
        _currentShader = shader;
    });
}

void CommandEncoderOpenGL::setFragmentTexture(const std::shared_ptr<Texture> &abstractTexture, size_t index)
{
    assert(abstractTexture);
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
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
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        auto sampler = std::dynamic_pointer_cast<TextureSamplerOpenGL>(abstractSampler);
        GLuint handle = sampler->getHandle();
        glBindSampler((GLuint)index, handle);
        CHECK_GL_ERROR();
    });
}

void CommandEncoderOpenGL::setVertexBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
{
    assert(abstractBuffer);
    
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        if (!_currentShader) {
            throw Exception("Must bind a shader before calling setVertexBuffer().");
        }
        
        auto buffer = std::dynamic_pointer_cast<BufferOpenGL>(abstractBuffer);
        const GLuint vbo = buffer->getHandleVBO();
        const GLenum target = buffer->getTargetEnum();
        
        if (UniformBuffer == buffer->getType()) {
            glBindBufferBase(target, 0, vbo);
        } else {
            GLuint vao = buffer->getHandleVAO();
            glBindVertexArray(vao);
            glBindBuffer(target, vbo);
            
            const VertexFormat &format = _currentShader->getVertexFormat();
            setupVertexAttributes(format);
            
            glBindBuffer(target, 0);
        }
        CHECK_GL_ERROR();
    });
}

void CommandEncoderOpenGL::setFragmentBuffer(const std::shared_ptr<Buffer> &abstractBuffer, size_t index)
{
    // OpenGL does not reserve separate namespaces for fragment buffer
    // indices and vertex buffer indices?
    setVertexBuffer(abstractBuffer, index);
}

void CommandEncoderOpenGL::drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances)
{
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        GLenum mode;
        
        switch (type) {
            case Triangles:
                mode = GL_TRIANGLES;
                break;
                
            case TriangleStrip:
                mode = GL_TRIANGLE_STRIP;
                break;
                
            default:
                throw Exception("Invalid primitive type %d in call to drawPrimitives.\n", (int)type);
        }
        
        CHECK_GL_ERROR();
        glDrawArrays(mode, (GLint)first, (GLsizei)count);
        CHECK_GL_ERROR();
    });
}

void CommandEncoderOpenGL::setTriangleFillMode(TriangleFillMode fillMode)
{
    GLenum glTriangleFillMode;
    
    switch (fillMode)
    {
        case Fill:
            glTriangleFillMode = GL_FILL;
            break;
            
        case Lines:
            glTriangleFillMode = GL_LINE;
            break;
            
        default:
            throw Exception("Unsupported triangle fill mode in setTriangleFillMode().");
    }
    
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [glTriangleFillMode]{
        CHECK_GL_ERROR();
        glPolygonMode(GL_FRONT_AND_BACK, glTriangleFillMode);
        CHECK_GL_ERROR();
    });
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

void CommandEncoderOpenGL::commit()
{
    _mainCommandQueue->enqueue(_encoderCommandQueue);
}

void CommandEncoderOpenGL::setDepthTest(bool enable)
{
    _encoderCommandQueue.enqueue(_id, __FUNCTION__, [=]{
        internalSetDepthTest(enable);
    });
}

void CommandEncoderOpenGL::internalSetDepthTest(bool enable)
{
    if (enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    } else {
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
    }
}
