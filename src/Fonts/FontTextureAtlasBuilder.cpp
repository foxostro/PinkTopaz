//
//  FontTextureAtlasBuilder.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Fonts/FontTextureAtlasBuilder.hpp"
#include "Fonts/FontTextureAtlasPacker.hpp"
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

FontTextureAtlasBuilder::FontTextureAtlasBuilder(std::shared_ptr<spdlog::logger> log,
                                                 const TextAttributes &attr)
: _log(log)
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
    FontTextureAtlasPacker packer;
    
    constexpr size_t initialAtlasSize = 160;
    constexpr size_t maxAtlasSize = 4096;
    SDL_Surface *atlasSurface = nullptr;
    
    // Render all the glyphs into separate surfaces.
    std::vector<std::shared_ptr<Glyph>> glyphs;
    for (FT_ULong charcode = 32; charcode < 127; ++charcode) {
        glyphs.push_back(glyphRenderer.render(charcode));
    }
    
    // Pack glyphs into a texture atlas. If we can't fit them all then we
    // increase the size and try again.
    for(size_t atlasSize = initialAtlasSize;
        !atlasSurface && atlasSize < maxAtlasSize;
        atlasSize += 4) {
        
        _log->info("Trying to create texture atlas of size {}.", atlasSize);
        
        auto maybePackedGlyphs = packer.packGlyphs(glyphs, atlasSize);
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

SDL_Surface*
FontTextureAtlasBuilder::createTextureAtlas(const std::vector<std::shared_ptr<Glyph>> &glyphs,
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
    
    for (const auto &glyph : glyphs) {
        const char charcode = glyph->getCharCode();
        auto iter = packedGlyphs.find(charcode);
        if (iter == packedGlyphs.end()) {
            throw Exception("Packed glyphs is missing charcode %c", charcode);
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
