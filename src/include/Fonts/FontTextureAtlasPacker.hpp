//
//  FontTextureAtlasPacker.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlasPacker_hpp
#define FontTextureAtlasPacker_hpp

#include "Fonts/PackedGlyph.hpp"
#include "Fonts/Glyph.hpp"
#include <unordered_map>
#include <boost/optional.hpp>

// Arranges glyphs to pack them into a texture atlas.
class FontTextureAtlasPacker
{
public:
    ~FontTextureAtlasPacker() = default;
    FontTextureAtlasPacker() = default;
    
    // Packs the glyphs into the a texture atlas of the specified size.
    // Returns `none' if this is not possible.
    boost::optional<std::unordered_map<char, PackedGlyph>>
    packGlyphs(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
               size_t atlasSize);
    
private:
    // Draw a glyph into the specified surface.
    // Returns `none' if this is not possible
    boost::optional<PackedGlyph>
    packGlyph(Glyph &glyph,
              size_t atlasSize,
              glm::ivec2 &cursor,
              size_t &rowHeight);
};

#endif /* FontTextureAtlasPacker_hpp */
