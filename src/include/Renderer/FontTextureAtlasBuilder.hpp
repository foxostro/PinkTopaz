//
//  FontTextureAtlasBuilder.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlasBuilder_hpp
#define FontTextureAtlasBuilder_hpp

#include "Renderer/GlyphRenderer.hpp"
#include "Renderer/PackedGlyph.hpp"

#include <glm/vec2.hpp>

#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

struct SDL_Surface;

// Creates a texture atlas which contains packed font glyphs for string drawing.
// Besides generating the image itself, this class also records metadata on how
// to access the atlas such as which texture coordinates correspond to which
// characters.
class FontTextureAtlasBuilder
{
public:
    ~FontTextureAtlasBuilder();
    
    // Build a texture atlas to accomodate text with the specified attributes.
    FontTextureAtlasBuilder(const TextAttributes &attributes);
    
    // Copy the texture atlas to a new SDL surface.
    SDL_Surface* copySurface();
    
    inline const std::unordered_map<char, PackedGlyph>& getGlyphs() const
    {
        return _glyphs;
    }
    
private:
    // Create a glypg renderer which can accomodate the specified attributes.
    std::shared_ptr<GlyphRenderer>
    makeGlyphRenderer(FT_Library &library,
                      FT_Face &face,
                      const TextAttributes &a);
    
    // Searches for, and returns, the smallest font texture atlas that can
    // accomodate the specified font at the specified font size.
    // When this method returns, `_glyphs' will contain valid glyph metrics.
    SDL_Surface* atlasSearch(GlyphRenderer &glyphRenderer);
    
    // Creates a font texture atlas with the specified character set.
    // The atlas size is directly specified. Though, this method will return
    // false if it is not possible to pack all characters into a surface of
    // this size.
    SDL_Surface*
    makeTextureAtlas(const std::vector<std::pair<char, std::shared_ptr<Glyph>>> &glyphs,
                     size_t atlasSize);
    
    // Draw a glyph into the specified surface.
    bool placeGlyph(Glyph &glyph,
                    SDL_Surface *atlasSurface,
                    glm::ivec2 &cursor,
                    size_t &rowHeight);
    
    SDL_Surface *_atlasSurface;
    std::unordered_map<char, PackedGlyph> _glyphs;
};

#endif /* FontTextureAtlasBuilder_hpp */
