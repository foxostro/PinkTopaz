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
#include <glm/vec4.hpp>

#include "Renderer/Shader.hpp"
#include "Renderer/TextureArray.hpp"
#include "Renderer/StaticMeshVao.hpp"

namespace PinkTopaz::Renderer {
    
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
        virtual void setFragmentTexture(const std::shared_ptr<TextureArray> &texture, size_t index) = 0;
        
        // Binds the specified VAO for use.
        // TODO: I'm not sure is the right approach. Take a look at the Metal documentation.
        virtual void setVertexArray(const std::shared_ptr<StaticMeshVao> &vao) = 0;
        
        // Draw triangle primitives using the bound buffers and other resources.
        virtual void drawTriangles(size_t first, size_t count) = 0;
    };

}; // namespace PinkTopaz::Renderer

#endif /* CommandEncoder_hpp */
