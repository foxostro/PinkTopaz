//
//  FontTextureAtlasBuilder.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/FontTextureAtlasBuilder.hpp"
#include "Renderer/GlyphRendererOutline.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include "SDL.h"
#include "SDL_image.h"

#include <fstream>
#include <cereal/types/unordered_map.hpp>
#include "CerealGLM.hpp"

FontTextureAtlasBuilder::~FontTextureAtlasBuilder()
{
    SDL_FreeSurface(_atlasSurface);
}

FontTextureAtlasBuilder::FontTextureAtlasBuilder(const TextAttributes &attr)
{
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        throw Exception("Failed to initialize Freetype.");
    }
    
    FT_Face face;
    if (FT_New_Face(library, attr.fontName.string().c_str(), 0, &face)) {
        throw Exception("Failed to load the font: %s", attr.fontName.c_str());
    }
    
    if (FT_Set_Pixel_Sizes(face, 0, attr.fontSize)) {
        throw Exception("Failed to set the font size.");
    }
    
    {
        GlyphRendererOutline glyphRenderer(library, face, attr);
        _atlasSurface = atlasSearch(face, glyphRenderer);
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

SDL_Surface*
FontTextureAtlasBuilder::atlasSearch(FT_Face &face,
                                     GlyphRenderer &glyphRenderer)
{
    constexpr size_t initialAtlasSize = 160;
    constexpr size_t maxAtlasSize = 4096;
    SDL_Surface *atlasSurface = nullptr;
    
    auto chars = getCharSet(face);
    
    for(size_t atlasSize = initialAtlasSize;
        !atlasSurface && atlasSize < maxAtlasSize;
        atlasSize += 32) {
        
        SDL_Log("Trying to create texture atlas of size %d", (int)atlasSize);
        atlasSurface = makeTextureAtlas(face, glyphRenderer, chars, atlasSize);
    }
    
    return atlasSurface;
}

SDL_Surface*
FontTextureAtlasBuilder::makeTextureAtlas(FT_Face &face,
                                          GlyphRenderer &glyphRenderer,
                                          const std::vector<std::pair<char, unsigned>> &characters,
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
    
    for (auto &character : characters) {
        char charcode = character.first;
        if (!placeGlyph(face, glyphRenderer, charcode,
                        atlasSurface, cursor, rowHeight)) {
            
            SDL_FreeSurface(atlasSurface);
            atlasSurface = nullptr;
            _glyphs.clear();
            break;
        }
    }
    
    return atlasSurface;
}

bool FontTextureAtlasBuilder::placeGlyph(FT_Face &face,
                                         GlyphRenderer &glyphRenderer,
                                         FT_ULong charcode,
                                         SDL_Surface *atlasSurface,
                                         glm::ivec2 &cursor,
                                         size_t &rowHeight)
{
    auto glyph = glyphRenderer.render(charcode);
    
    rowHeight = std::max(rowHeight, (size_t)glyph->getSize().y);
    
    // Validate the cursor. Can the glyph fit on this row?
    if ((cursor.x + glyph->getSize().x) >= (size_t)atlasSurface->w) {
        // Go to the next row.
        cursor.x = 0;
        cursor.y += rowHeight;
        rowHeight = glyph->getSize().y;
        
        // Have we run out of rows? If so then try a bigger atlas.
        if ((cursor.y + glyph->getSize().y) >= (size_t)atlasSurface->h) {
            return false;
        }
    }
    
    glyph->blit(atlasSurface, cursor);
    
    // Now store the packed glyph for later use.
    _glyphs[(char)charcode] = (PackedGlyph){
        glm::vec2((float)cursor.x / atlasSurface->w,
                  (float)cursor.y / atlasSurface->w),
        glm::vec2((float)glyph->getSize().x / atlasSurface->h,
                  (float)glyph->getSize().y / atlasSurface->h),
        glyph->getSize(),
        glyph->getBearing(),
        glyph->getAdvance()
    };
    
    // Increment the cursor. We've already validated for this glyph.
    cursor.x += glyph->getSize().x;
    
    return true;
}

std::vector<std::pair<char, unsigned>>
FontTextureAtlasBuilder::getCharSet(FT_Face &face)
{
    std::vector<std::pair<char, unsigned>> characters;
    
    for (FT_ULong c = 32; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            throw Exception("Failed to load the glyph %c.", (char)c);
        }
        
        unsigned height = face->glyph->bitmap.rows;
        characters.emplace_back(std::make_pair((char)c, height));
    }
    
    std::sort(characters.begin(), characters.end(),
              [](const std::pair<char, int> &left,
                 const std::pair<char, int> &right) -> bool {
                  return (left.second < right.second);
              });
    
    return characters;
}
