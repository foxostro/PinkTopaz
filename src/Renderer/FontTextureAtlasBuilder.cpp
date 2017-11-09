//
//  FontTextureAtlasBuilder.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/FontTextureAtlasBuilder.hpp"
#include "Renderer/GlyphRendererOutline.hpp"
#include "Renderer/GlyphRendererRegular.hpp"
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
    
    // Pack glyphs into a texture atlas. If we can't fit them all then we
    // increase the size and try again.
    for(size_t atlasSize = initialAtlasSize;
        !atlasSurface && atlasSize < maxAtlasSize;
        atlasSize += 32) {
        
        SDL_Log("Trying to create texture atlas of size %d", (int)atlasSize);
        atlasSurface = makeTextureAtlas(sortedGlyphs, atlasSize);
    }
    
    return atlasSurface;
}

SDL_Surface*
FontTextureAtlasBuilder::makeTextureAtlas(const std::vector<std::pair<char, std::shared_ptr<Glyph>>> &glyphs,
                                          size_t size)
{
    _glyphs.clear();
    
    size_t rowHeight = 0;
    glm::ivec2 cursor(0, 0);
    
    SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0,
                                                     (int)size, (int)size,
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
        if (!placeGlyph(*glyph, atlasSurface, cursor, rowHeight)) {
            
            SDL_FreeSurface(atlasSurface);
            atlasSurface = nullptr;
            _glyphs.clear();
            break;
        }
    }
    
    return atlasSurface;
}

bool FontTextureAtlasBuilder::placeGlyph(Glyph &glyph,
                                         SDL_Surface *atlasSurface,
                                         glm::ivec2 &cursor,
                                         size_t &rowHeight)
{
    rowHeight = std::max(rowHeight, (size_t)glyph.getSize().y);
    
    // Validate the cursor. Can the glyph fit on this row?
    if ((cursor.x + glyph.getSize().x) >= (size_t)atlasSurface->w) {
        // Go to the next row.
        cursor.x = 0;
        cursor.y += rowHeight;
        rowHeight = glyph.getSize().y;
        
        // Have we run out of rows? If so then try a bigger atlas.
        if ((cursor.y + glyph.getSize().y) >= (size_t)atlasSurface->h) {
            return false;
        }
    }
    
    glyph.blit(atlasSurface, cursor);
    
    // Now store the packed glyph for later use.
    _glyphs[glyph.getCharCode()] = (PackedGlyph){
        glm::vec2((float)cursor.x / atlasSurface->w,
                  (float)cursor.y / atlasSurface->w),
        glm::vec2((float)glyph.getSize().x / atlasSurface->h,
                  (float)glyph.getSize().y / atlasSurface->h),
        glyph.getSize(),
        glyph.getBearing(),
        glyph.getAdvance()
    };
    
    // Increment the cursor. We've already validated for this glyph.
    cursor.x += glyph.getSize().x;
    
    return true;
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
