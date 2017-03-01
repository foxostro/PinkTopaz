//
//  StringRenderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef StringRenderer_hpp
#define StringRenderer_hpp

#include "Renderer/String.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/GraphicsDevice.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <map>

namespace PinkTopaz::Renderer {
    
    class StringRenderer
    {
    public:
        StringRenderer(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                       const std::string &fontName,
                       unsigned fontSize);
        ~StringRenderer() = default;
        
        void draw(const glm::ivec4 &viewport);
        
    private:
        struct Glyph
        {
            std::shared_ptr<Texture> texture;
            glm::ivec2 size;
            glm::ivec2 bearing;
            unsigned advance; // Given in 1/64 points.
        };
        
        void drawString(const std::shared_ptr<CommandEncoder> &encoder,
                        const std::string &text,
                        glm::vec2 basePos,
                        float scale,
                        const glm::vec3 &color);
        
        std::shared_ptr<GraphicsDevice> _graphicsDevice;
        std::map<char, Glyph> _glyphs;
        std::shared_ptr<Shader> _shader;
        std::shared_ptr<Buffer> _buffer;
        RenderPassDescriptor _renderPassDescriptor;
        
        const glm::ivec2 _canvasSize;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StringRenderer_hpp */
