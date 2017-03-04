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
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/RenderPassDescriptor.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class CommandEncoderOpenGL : public CommandEncoder
    {
    public:
        CommandEncoderOpenGL(const RenderPassDescriptor &desc);
        
        void setViewport(const glm::ivec4 &viewport) override;
        void setShader(const std::shared_ptr<Shader> &shader) override;
        void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) override;
        void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) override;
        void setVertexBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
        void setFragmentBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) override;
        void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) override;
        void updateFence(const std::shared_ptr<Fence> &fence) override;
        void waitForFence(const std::shared_ptr<Fence> &fence,
                          std::function<void()> &&completionHandler) override;
        
        inline CommandQueue& getCommandQueue() { return _commandQueue; }
        
    private:
        CommandQueue _commandQueue;
    };
    
}; // namespace PinkTopaz::Renderer::OpenGL

#endif /* CommandEncoderOpenGL_hpp */
