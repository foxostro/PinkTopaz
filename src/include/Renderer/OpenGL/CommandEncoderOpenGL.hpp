//
//  CommandEncoderOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandEncoderOpenGL_hpp
#define CommandEncoderOpenGL_hpp

#include <memory>
#include <string>
#include <mutex>
#include <queue>
#include <glm/vec4.hpp>

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
        void setVertexBuffer(const std::shared_ptr<Buffer> &buffer) override;
        void setVertexBytes(const std::shared_ptr<Buffer> &abstractBuffer, size_t size, const void *data) override;
        void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) override;
        
        inline CommandQueue& getCommandQueue() { return _commandQueue; }
        
    private:
        CommandQueue _commandQueue;
    };
    
}; // namespace PinkTopaz::Renderer::OpenGL

#endif /* CommandEncoderOpenGL_hpp */
