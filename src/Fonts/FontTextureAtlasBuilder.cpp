//
//  FontTextureAtlasBuilder.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Fonts/FontTextureAtlasBuilder.hpp"
#include "Fonts/GlyphRendererOutline.hpp"
#include "Fonts/GlyphRendererRegular.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include "SDL.h"
#include "SDL_image.h"

#include <fstream>
#include <cereal/types/unordered_map.hpp>
#include "CerealGLM.hpp"

#include <boost/algorithm/string/find.hpp>

FontTextureAtlasBuilder::~FontTextureAtlasBuilder()
{
    SDL_FreeSurface(_atlasSurface);
}

FontTextureAtlasBuilder::FontTextureAtlasBuilder(const TextAttributes &attr)
{
    auto maybeFontPath = getFontPath(attr);
    if (!maybeFontPath) {
        throw Exception("Failed to find font file for \"%s\"",
                        attr.fontName.c_str());
    }
    
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        throw Exception("Failed to initialize Freetype.");
    }
    
    FT_Face face;
    if (FT_New_Face(library, maybeFontPath->string().c_str(), 0, &face)) {
        throw Exception("Failed to load the font: %s", attr.fontName.c_str());
    }
    
    if (FT_Set_Pixel_Sizes(face, 0, attr.fontSize)) {
        throw Exception("Failed to set the font size.");
    }
    
    {
        std::shared_ptr<GlyphRenderer> glyphRenderer = makeGlyphRenderer(library, face, attr);
        _atlasSurface = atlasSearch(*glyphRenderer);
        if (!_atlasSurface) {
            throw Exception("Failed to generate font texture atlas.");
        }
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

SDL_Surface* FontTextureAtlasBuilder::copySurface()
{
    return SDL_ConvertSurface(_atlasSurface,
                              _atlasSurface->format,
                              SDL_SWSURFACE);
}

std::shared_ptr<GlyphRenderer>
FontTextureAtlasBuilder::makeGlyphRenderer(FT_Library &library,
                                           FT_Face &face,
                                           const TextAttributes &attr)
{
    if (attr.border == 0) {
        return std::dynamic_pointer_cast<GlyphRenderer>(std::make_shared<GlyphRendererRegular>(library, face, attr));
    } else {
        return std::dynamic_pointer_cast<GlyphRenderer>(std::make_shared<GlyphRendererOutline>(library, face, attr));
    }
}

SDL_Surface*
FontTextureAtlasBuilder::atlasSearch(GlyphRenderer &glyphRenderer)
{
    constexpr size_t initialAtlasSize = 160;
    constexpr size_t maxAtlasSize = 4096;
    SDL_Surface *atlasSurface = nullptr;
    
    // Render all the glyphs into separate surfaces.
    std::unordered_map<char, std::shared_ptr<Glyph>> glyphs;
    for (FT_ULong charcode = 32; charcode < 127; ++charcode) {
        glyphs[(char)charcode] = glyphRenderer.render(charcode);
    }
    
    // Pack glyphs into a texture atlas. If we can't fit them all then we
    // increase the size and try again.
    for(size_t atlasSize = initialAtlasSize;
        !atlasSurface && atlasSize < maxAtlasSize;
        atlasSize += 32) {
        
        SDL_Log("Trying to create texture atlas of size %d", (int)atlasSize);
        
        auto maybePackedGlyphs = packGlyphs(glyphs, atlasSize);
        if (maybePackedGlyphs) {
            // Now that glyphs have been arranged and packed, render them to
            // a surface to create the texture atlas.
            _glyphs = *maybePackedGlyphs;
            atlasSurface = createTextureAtlas(glyphs, *maybePackedGlyphs,
                                              atlasSize);
        }
    }
    
    return atlasSurface;
}

boost::optional<std::unordered_map<char, PackedGlyph>>
FontTextureAtlasBuilder::packGlyphs(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
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
FontTextureAtlasBuilder::packGlyph(Glyph &glyph,
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

SDL_Surface*
FontTextureAtlasBuilder::createTextureAtlas(const std::unordered_map<char, std::shared_ptr<Glyph>> &glyphs,
                                            const std::unordered_map<char, PackedGlyph> &packedGlyphs,
                                            size_t atlasSize)
{
    SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0,
                                                     (int)atlasSize,
                                                     (int)atlasSize,
                                                     (int)(sizeof(uint32_t)*8),
                                                     0x000000ff,
                                                     0x0000ff00,
                                                     0x00ff0000,
                                                     0xff000000);
    
    if (!atlasSurface) {
        throw Exception("Failed to create SDL surface for font texture atlas");
    }
    
    if (SDL_SetSurfaceBlendMode(atlasSurface, SDL_BLENDMODE_BLEND)) {
        throw Exception("Failed to set font texture atlas blend mode.");
    }
    
    for (auto& [charcode, glyph] : glyphs) {
        auto iter = packedGlyphs.find(charcode);
        if (iter == packedGlyphs.end()) {
            const char c = charcode;
            throw Exception("Packed glyphs is missing charcode %c", c);
        }
        
        const glm::ivec2 cursor(iter->second.uvOrigin.x * atlasSurface->w,
                                iter->second.uvOrigin.y * atlasSurface->h);
        glyph->blit(atlasSurface, cursor);
    }
    
    return atlasSurface;
}

boost::optional<boost::filesystem::path>
FontTextureAtlasBuilder::getFontPath(const TextAttributes &attr)
{
    std::string weight;
    
    switch (attr.weight) {
        case Light:
            weight = "Light";
            break;
            
        case Regular:
            weight = "Regular";
            break;
            
        case Bold:
            weight = "Bold";
            break;
            
        default:
            assert(!"unreachable");
    }
    
    boost::filesystem::path directory(".");
    boost::filesystem::recursive_directory_iterator iter(directory);
    
    for (const boost::filesystem::path &path : iter) {
        if (boost::filesystem::is_regular_file(path)
            && path.string().find(attr.fontName) != std::string::npos
            && path.string().find(weight) != std::string::npos) {
            
            return boost::make_optional(path);
        }
    }
    
    return boost::none;
}
