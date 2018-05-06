//
//  CommandEncoderOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandEncoderOpenGL_hpp
#define CommandEncoderOpenGL_hpp

#include "Renderer/CommandEncoder.hpp"
#include "Renderer/RenderPassDescriptor.hpp"
#include "Renderer/OpenGL/ShaderOpenGL.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

class CommandEncoderOpenGL : public CommandEncoder
{
public:
    CommandEncoderOpenGL(unsigned id,
                         const std::shared_ptr<CommandQueue> &commandQueue,
                         const RenderPassDescriptor &desc);
    
    void setViewport(const glm::ivec4 &viewport) override;
    void setShader(const std::shared_ptr<Shader> &shader) override;
    void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) override;
    void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) override;
    void setVertexBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
    void setFragmentBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
    void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) override;
    void drawIndexedPrimitives(PrimitiveType type, size_t indexCount, const std::shared_ptr<Buffer> &indexBuffer, size_t instanceCount) override;
    void setTriangleFillMode(TriangleFillMode fillMode) override;
    void commit() override;
    void setDepthTest(bool enable) override;
    
private:
    // Tell OpenGL about the vertex attribute format expected by the shader.
    // Call this after the VAO and VBO have both been bound.
    static void setupVertexAttributes(const VertexFormat &format);
    
    void internalSetDepthTest(bool enable);
    
    unsigned _id;
    std::shared_ptr<ShaderOpenGL> _currentShader;
    std::shared_ptr<CommandQueue> _mainCommandQueue;
    CommandQueue _encoderCommandQueue;
};

#endif /* CommandEncoderOpenGL_hpp */
