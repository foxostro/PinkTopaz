//
//  FontTextureAtlas.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlas_hpp
#define FontTextureAtlas_hpp

#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextAttributes.hpp"
#include "Renderer/PackedGlyph.hpp"

#include <glm/vec2.hpp>
#include <unordered_map>
#include <boost/optional.hpp>

// Builds a GPU texture for the font texture atlas. The font texture atlas is
// either built from scratch or retrieved from cache.
class FontTextureAtlas
{
public:
    ~FontTextureAtlas() = default;
    
    FontTextureAtlas(GraphicsDevice &graphicsDevice,
                     const TextAttributes &attributes);
    
    // Get the glyph for the specified character code.
    boost::optional<PackedGlyph> getGlyph(char c) const;
    
    // Get the texture atlas.
    inline std::shared_ptr<Texture> getTexture()
    {
        return _textureAtlas;
    }
    
private:
    std::shared_ptr<Texture> _textureAtlas;
    std::unordered_map<char, PackedGlyph> _glyphs;
};

#endif /* FontTextureAtlas_hpp */
