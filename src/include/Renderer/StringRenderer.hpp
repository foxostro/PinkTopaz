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
#include "Renderer/TextureSampler.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/GraphicsDevice.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <map>
#include <list>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SDL.h"

namespace PinkTopaz::Renderer {
    
    class StringRenderer
    {
    public:
        typedef std::list<String>::iterator StringHandle;
        
        StringRenderer(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                       const std::string &fontName,
                       unsigned fontSize);
        ~StringRenderer() = default;
        
        // Draws all registered strings.
        void draw(const glm::ivec4 &viewport);
        
        // Add a string to be rendered.
        StringHandle add(const std::string &contents,
                         const glm::vec2 &position,
                         const glm::vec3 &color);
        
        // Remove a string that was previously added.
        void remove(StringHandle &handle);
        
        // Change the contents of a string that was previously added.
        void replaceContents(StringHandle &handle, const std::string &contents);
        
    private:
        struct Glyph
        {
            glm::vec2 uvOrigin;
            glm::vec2 uvExtent;
            glm::ivec2 size;
            glm::ivec2 bearing;
            unsigned advance; // Given in 1/64 points.
        };
        
        static std::vector<uint8_t> getGrayScaleImageBytes(SDL_Surface *surface);
        static bool placeGlyph(FT_Face &face,
                               FT_ULong c,
                               SDL_Surface *atlasSurface,
                               std::map<char, Glyph> &glyphs,
                               glm::ivec2 &cursor,
                               size_t &rowHeight);
        SDL_Surface* atlasSearch(FT_Face &face, unsigned fontSize);
        SDL_Surface* makeTextureAtlas(FT_Face &face, size_t atlasSize);
        std::shared_ptr<Texture> makeTextureAtlas(const std::string &fontName,
                                                  unsigned fontSize);
        void rebuildBuffer(String &string);
        
        std::shared_ptr<GraphicsDevice> _graphicsDevice;
        std::shared_ptr<Texture> _textureAtlas;
        std::map<char, Glyph> _glyphs;
        std::shared_ptr<Shader> _shader;
        std::shared_ptr<TextureSampler> _sampler;
        VertexFormat _vertexFormat;
        RenderPassDescriptor _renderPassDescriptor;
        std::list<String> _strings;
        glm::ivec2 _canvasSize;
        glm::ivec4 _viewport;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StringRenderer_hpp */
