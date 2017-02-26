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
#include "Renderer/StaticMeshVao.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class CommandEncoderOpenGL : public CommandEncoder
    {
    public:
        void setViewport(const glm::ivec4 &viewport) override;
        void setShader(const std::shared_ptr<Shader> &shader) override;
        void setFragmentTexture(const std::shared_ptr<TextureArray> &texture, size_t index) override;
        void setVertexArray(const std::shared_ptr<StaticMeshVao> &vao) override;
        void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) override;
        
        inline CommandQueue& getCommandQueue() { return _commandQueue; }
        
    private:
        CommandQueue _commandQueue;
    };
    
}; // namespace PinkTopaz::Renderer::OpenGL

#endif /* CommandEncoderOpenGL_hpp */
