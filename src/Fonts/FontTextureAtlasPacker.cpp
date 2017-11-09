//
//  FontTextureAtlasPacker.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Fonts/FontTextureAtlasPacker.hpp"
#include <vector>

boost::optional<std::unordered_map<char, PackedGlyph>>
FontTextureAtlasPacker::packGlyphs(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
                                   size_t size)
{
    // Sort glyphs by height.
    std::vector<std::pair<char, std::shared_ptr<Glyph>>> sortedGlyphs;
    for (auto pair : glyphs) {
        sortedGlyphs.emplace_back(pair);
    }
    std::sort(sortedGlyphs.begin(), sortedGlyphs.end(),
              [](const std::pair<char, std::shared_ptr<Glyph>> &left,
                 const std::pair<char, std::shared_ptr<Glyph>> &right) -> bool {
                  return (left.second->getSize().y < right.second->getSize().y);
              });
    
    std::unordered_map<char, PackedGlyph> packedGlyphs;
    
    size_t rowHeight = 0;
    glm::ivec2 cursor(0, 0);
    
    for (auto& [charcode, glyph] : glyphs) {
        auto maybePackedGlyph = packGlyph(*glyph, size, cursor, rowHeight);
        if (maybePackedGlyph) {
            packedGlyphs[charcode] = *maybePackedGlyph;
        } else {
            return boost::none;
        }
    }
    
    return packedGlyphs;
}

boost::optional<PackedGlyph>
FontTextureAtlasPacker::packGlyph(Glyph &glyph,
                                  size_t atlasSize,
                                  glm::ivec2 &cursor,
                                  size_t &rowHeight)
{
    rowHeight = std::max(rowHeight, (size_t)glyph.getSize().y);
    
    // Validate the cursor. Can the glyph fit on this row?
    if ((cursor.x + glyph.getSize().x) >= (size_t)atlasSize) {
        // Go to the next row.
        cursor.x = 0;
        cursor.y += rowHeight;
        rowHeight = glyph.getSize().y;
        
        // Have we run out of rows? If so then try a bigger atlas.
        if ((cursor.y + glyph.getSize().y) >= (size_t)atlasSize) {
            return boost::none;
        }
    }
    
    // Now store the packed glyph for later use.
    PackedGlyph packedGlyph = {
        glm::vec2((float)cursor.x / atlasSize,
                  (float)cursor.y / atlasSize),
        glm::vec2((float)glyph.getSize().x / atlasSize,
                  (float)glyph.getSize().y / atlasSize),
        glyph.getSize(),
        glyph.getBearing(),
        glyph.getAdvance()
    };
    
    // Increment the cursor. We've already validated for this glyph.
    cursor.x += glyph.getSize().x;
    
    return boost::make_optional(packedGlyph);
}
