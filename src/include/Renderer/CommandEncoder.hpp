//
//  CommandEncoder.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandEncoder_hpp
#define CommandEncoder_hpp

#include <memory>
#include <functional>
#include <glm/vec4.hpp>

#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"
#include "Renderer/Buffer.hpp"

namespace PinkTopaz::Renderer {
    
    enum PrimitiveType
    {
        Triangles
    };
    
    class CommandEncoder
    {
    public:
        virtual ~CommandEncoder() {}
        
        // Sets the rendering viewport. This is necessary for computing screen
        // space coordinates. The vector contains the viewport origin in xy and
        // the viewport size in zw.
        virtual void setViewport(const glm::ivec4 &viewport) = 0;
        
        // Binds the specified shader program for use.
        virtual void setShader(const std::shared_ptr<Shader> &shader) = 0;
        
        // Use the texture at the specified index in the fragment program.
        virtual void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) = 0;
        
        // Use the texture sampler at the specified index in the fragment program.
        virtual void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) = 0;
        
        // Binds the specified buffer for use in the vertex program.
        virtual void setVertexBuffer(const std::shared_ptr<Buffer> &buffer) = 0;
        
        // Sets a block of data for the vertex shader.
        virtual void setVertexBytes(const std::shared_ptr<Buffer> &abstractBuffer, size_t size, const void *data) = 0;
        
        // Draw triangle primitives using the bound buffers and other resources.
        virtual void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) = 0;
    };

}; // namespace PinkTopaz::Renderer

#endif /* CommandEncoder_hpp */
