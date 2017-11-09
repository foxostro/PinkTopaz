//
//  FontTextureAtlasBuilder.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlasBuilder_hpp
#define FontTextureAtlasBuilder_hpp

#include "Fonts/GlyphRenderer.hpp"
#include "Fonts/PackedGlyph.hpp"

#include <glm/vec2.hpp>

#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

struct SDL_Surface;

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

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
    
    // Packs the glyphs into the a texture atlas of the specified size.
    // Returns `none' if this is not possible.
    boost::optional<std::unordered_map<char, PackedGlyph>>
    packGlyphs(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
               size_t atlasSize);
    
    // Draw a glyph into the specified surface.
    // Returns `none' if this is not possible
    boost::optional<PackedGlyph>
    packGlyph(Glyph &glyph,
              size_t atlasSize,
              glm::ivec2 &cursor,
              size_t &rowHeight);
    
    // Create an SDL surface containing all the glyphs.
    SDL_Surface*
    createTextureAtlas(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
                       const std::unordered_map<char, PackedGlyph> &packedGlyphs,
                       size_t atlasSize);
    
    // Finds the path to the font file satsifying the specified attributes.
    boost::optional<boost::filesystem::path>
    getFontPath(const TextAttributes &attr);
    
    SDL_Surface *_atlasSurface;
    std::unordered_map<char, PackedGlyph> _glyphs;
};

#endif /* FontTextureAtlasBuilder_hpp */
