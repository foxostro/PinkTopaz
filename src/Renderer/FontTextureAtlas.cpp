//
//  FontTextureAtlas.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/FontTextureAtlas.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include "SDL.h"
#include "SDL_image.h"

#include <fstream>
#include <cereal/types/unordered_map.hpp>
#include "CerealGLM.hpp"

static constexpr bool FORCE_REBUILD = true;
static constexpr int border = 2;

FontTextureAtlas::FontTextureAtlas(GraphicsDevice &graphicsDevice,
                                   const FontAttributes &attributes)
{
    const std::string baseName = "font" + std::to_string(attributes.fontSize);
    const boost::filesystem::path prefPath = getPrefPath();
    const boost::filesystem::path atlasImageFilename = prefPath / (baseName + ".png");
    const boost::filesystem::path atlasDictionaryFilename = prefPath / (baseName + ".cereal");
    
    SDL_Surface *atlasSurface = nullptr;
    
    // Font texture atlas is cached between runs of the game.
    if (!FORCE_REBUILD &&
        boost::filesystem::exists(atlasImageFilename) &&
        boost::filesystem::exists(atlasDictionaryFilename)) {
        
        SDL_Log("Loading font texture atlas from files: \"%s\" and \"%s\"",
                atlasImageFilename.string().c_str(),
                atlasDictionaryFilename.string().c_str());
        
        atlasSurface = IMG_Load(atlasImageFilename.string().c_str());
        
        std::ifstream is(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryInputArchive archive(is);
        archive(_glyphs);
    } else {
        atlasSurface = genTextureAtlas(attributes);
        
        SDL_Log("Saving font texture atlas to files: \"%s\" and \"%s\"",
                atlasImageFilename.string().c_str(),
                atlasDictionaryFilename.string().c_str());
        
        IMG_SavePNG(atlasSurface, atlasImageFilename.string().c_str());
        
        std::ofstream os(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryOutputArchive archive(os);
        archive(_glyphs);
    }
    
    // Turn the SDL surface into a texture.
    TextureDescriptor texDesc = {
        Texture2D,                            // type
        RGBA8,                                // format
        static_cast<size_t>(atlasSurface->w), // width
        static_cast<size_t>(atlasSurface->h), // height
        1,                                    // depth
        4,                                    // unpackAlignment
        false,                                // generateMipMaps
    };
    _textureAtlas = graphicsDevice.makeTexture(texDesc, atlasSurface->pixels);
    SDL_FreeSurface(atlasSurface);
}

std::vector<uint8_t> FontTextureAtlas::getGrayScaleImageBytes(SDL_Surface *surface)
{
    const size_t w = surface->w;
    const size_t h = surface->h;
    const uint32_t mask = surface->format->Rmask;
    const uint8_t shift = surface->format->Rshift;
    
    // We only want to store the RED components in the GPU texture.
    std::vector<uint8_t> atlasPixels(w * h);
    
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }
    
    uint32_t *srcRow = (uint32_t *)surface->pixels;
    uint8_t *dstRow = (uint8_t *)&atlasPixels[0];
    for(size_t y = 0; y < h; ++y)
    {
        for(size_t x = 0; x < w; ++x)
        {
            uint32_t pixel = srcRow[x];
            uint8_t component = (pixel & mask) >> shift;
            dstRow[x] = component;
        }
        
        srcRow += surface->pitch / surface->format->BytesPerPixel;
        dstRow += w;
    }
    
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
    
    return atlasPixels;
}

FT_Glyph
FontTextureAtlas::getGlyph(FT_Face &face,
                           FT_Stroker &stroker,
                           FT_ULong charcode,
                           GlyphStyle style)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(face, charcode);
    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT)) {
        throw Exception("Failed to load the glyph \"%c\"", (char)charcode);
    }
    
    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph)) {
        throw Exception("Failed to get the glyph \"%c\"", (char)charcode);
    }
    
    switch (style) {
        case INTERIOR:
            if (FT_Glyph_StrokeBorder(&glyph, stroker, true, true)) {
                FT_Done_Glyph(glyph);
                throw Exception("Failed to stroke the inside of the glyph \"%c\"", (char)charcode);
            }
            break;
            
        case BORDER:
            if (FT_Glyph_Stroke(&glyph, stroker, true)) {
                FT_Done_Glyph(glyph);
                throw Exception("Failed to stroke the border of the glyph \"%c\"", (char)charcode);
            }
            break;
            
        default:
            assert(!"not reachable");
    }
    
    return glyph;
}

void FontTextureAtlas::blitGlyph(SDL_Surface *atlasSurface,
                                 FT_Bitmap &bitmap,
                                 const glm::ivec2 &cursor,
                                 const glm::vec4 &color,
                                 BlitMode mode)
{
    assert(atlasSurface);
    
    const size_t width = bitmap.width;
    const size_t height = bitmap.rows;
    const size_t n = width * height;
    const uint8_t *glyphBytes = bitmap.buffer;
    
    // Convert grayscale image to RGBA so we can use SDL_Surface.
    std::vector<uint32_t> pixels(n);
    const uint8_t rshift = atlasSurface->format->Bshift;
    const uint8_t gshift = atlasSurface->format->Gshift;
    const uint8_t bshift = atlasSurface->format->Rshift;
    const uint8_t ashift = atlasSurface->format->Ashift;
    
    switch (mode) {
        case BLEND:
            for (size_t i = 0; i < n; ++i) {
                const float luminance = (float)glyphBytes[i] / 255.f;
                const float r = color.r * luminance;
                const float g = color.g * luminance;
                const float b = color.b * luminance;
                const float a = color.a * luminance;
                const uint32_t redComp   = r * 255.f;
                const uint32_t greenComp = g * 255.f;
                const uint32_t blueComp  = b * 255.f;
                const uint32_t alphaComp = a * 255.f;
                pixels[i] = (redComp   << rshift)
                | (greenComp << gshift)
                | (blueComp  << bshift)
                | (alphaComp << ashift);
            }
            break;
            
        case COLOR_KEY:
            for (size_t i = 0; i < n; ++i) {
                const uint8_t ilum = glyphBytes[i];
                if (ilum == 0) {
                    pixels[i] = 0;
                } else {
                    const float luminance = (float)ilum / 255.f;
                    const float r = color.r * luminance;
                    const float g = color.g * luminance;
                    const float b = color.b * luminance;
                    const float a = color.a;
                    const uint32_t redComp   = r * 255.f;
                    const uint32_t greenComp = g * 255.f;
                    const uint32_t blueComp  = b * 255.f;
                    const uint32_t alphaComp = a * 255.f;
                    pixels[i] = (redComp   << rshift)
                    | (greenComp << gshift)
                    | (blueComp  << bshift)
                    | (alphaComp << ashift);
                }
            }
            break;
            
        default:
            assert(!"unreachable");
    }
    
    // Create a surface with the glpyh image.
    SDL_Surface *glyphSurface = SDL_CreateRGBSurfaceFrom(&pixels[0],
                                                         (int)width,
                                                         (int)height,
                                                         (int)(sizeof(uint32_t) * 8),
                                                         (int)(width * sizeof(uint32_t)),
                                                         0x000000ff,
                                                         0x0000ff00,
                                                         0x00ff0000,
                                                         0xff000000);
    
    if (!glyphSurface) {
        throw Exception("Failed to create SDL surface for glyph");
    }
    
    if (SDL_SetSurfaceBlendMode(glyphSurface, SDL_BLENDMODE_BLEND)) {
        throw Exception("Failed to set font texture atlas blend mode.");
    }
    
    // Blit the glyph into the texture atlas at the cursor position.
    SDL_Rect src = {
        0, 0,
        (int)width, (int)height,
    };
    
    SDL_Rect dst = {
        cursor.x, cursor.y,
        (int)width, (int)height,
    };
    
    if (SDL_BlitSurface(glyphSurface, &src, atlasSurface, &dst)) {
        throw Exception("Failed to blit glpyh into font texture atlas surface.");
    }
    
    SDL_FreeSurface(glyphSurface);
}

bool FontTextureAtlas::placeGlyph(FT_Face &face,
                                  FT_Stroker &stroker,
                                  FT_ULong c,
                                  SDL_Surface *atlasSurface,
                                  glm::ivec2 &cursor,
                                  size_t &rowHeight)
{
    FT_Glyph glyphBorder = getGlyph(face, stroker, c, BORDER);
    FT_Glyph_To_Bitmap(&glyphBorder, FT_RENDER_MODE_NORMAL, nullptr, true);
    assert(glyphBorder->format == FT_GLYPH_FORMAT_BITMAP);
    FT_BitmapGlyph bitmapGlyphBorder = (FT_BitmapGlyph)glyphBorder;
    FT_Bitmap &bitmapBorder = bitmapGlyphBorder->bitmap;

    FT_Glyph glyphInterior = getGlyph(face, stroker, c, INTERIOR);
    FT_Glyph_To_Bitmap(&glyphInterior, FT_RENDER_MODE_NORMAL, nullptr, true);
    assert(glyphInterior->format == FT_GLYPH_FORMAT_BITMAP);
    FT_BitmapGlyph bitmapGlyphInterior = (FT_BitmapGlyph)glyphInterior;
    
    rowHeight = std::max(rowHeight, (size_t)bitmapBorder.rows);
    
    // Validate the cursor. Can the glyph fit on this row?
    if ((cursor.x + bitmapBorder.width) >= (size_t)atlasSurface->w) {
        // Go to the next row.
        cursor.x = 0;
        cursor.y += rowHeight;
        rowHeight = bitmapBorder.rows;
        
        // Have we run out of rows? If so then try a bigger atlas.
        if ((cursor.y + bitmapBorder.rows) >= (size_t)atlasSurface->h) {
            return false;
        }
    }
    
    // To achieve the outline, we render the interior first and render the
    // border second. The two layers must be aligned using the `left' and `top'
    // properties of the glyphs because the bitmaps are different sizes.
    //
    // Additionally, as FreeType doesn't give us alpha values in glyph bitmaps,
    // we must take care when creating assumed alpha values from the gray scale
    // bitmap. For the interior, we assume that black means clear and everything
    // else uses the alpha in the specified interior color. For the exterior,
    // we assume that alpha scales with luminance and blend when we blit. The
    // outer edge of the border thus blends with the surface background color
    // and gives the glyph a soft outer edge. The inner edge of the border
    // blends with the fully opaque values from the interior bitmap and the
    // result is a fully opaque inner edge.
    
    const glm::ivec2 offset(bitmapGlyphInterior->left - bitmapGlyphBorder->left,
                            bitmapGlyphBorder->top - bitmapGlyphInterior->top);
    
    blitGlyph(atlasSurface,
              bitmapGlyphInterior->bitmap,
              cursor + offset,
              glm::vec4(0.3f, 0.3f, 0.3f, 1.0f),
              COLOR_KEY);
    
    blitGlyph(atlasSurface,
              bitmapGlyphBorder->bitmap,
              cursor,
              glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
              BLEND);
    
    // Now store the glyph for later use.
    _glyphs[(char)c] = (Glyph){
        glm::vec2((float)cursor.x / atlasSurface->w,
                  (float)cursor.y / atlasSurface->w),
        glm::vec2((float)bitmapBorder.width / atlasSurface->h,
                  (float)bitmapBorder.rows / atlasSurface->h),
        glm::ivec2(bitmapBorder.width, bitmapBorder.rows),
        glm::ivec2(face->glyph->bitmap_left,
                   face->glyph->bitmap_top),
        (unsigned)face->glyph->advance.x
    };
    
    // Increment the cursor. We've already validated for this glyph.
    cursor.x += bitmapBorder.width;
    
    FT_Done_Glyph(glyphInterior);
    FT_Done_Glyph(glyphBorder);
    
    return true;
}

SDL_Surface*
FontTextureAtlas::makeTextureAtlas(FT_Face &face,
                                   FT_Stroker &stroker,
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
        char c = character.first;
        if (!placeGlyph(face, stroker, c, atlasSurface, cursor, rowHeight)) {
            SDL_FreeSurface(atlasSurface);
            atlasSurface = nullptr;
            _glyphs.clear();
            break;
        }
    }
    
    return atlasSurface;
}

std::vector<std::pair<char, unsigned>>
FontTextureAtlas::getCharSet(FT_Face &face)
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

SDL_Surface*
FontTextureAtlas::atlasSearch(FT_Face &face,
                              FT_Stroker &stroker,
                              unsigned fontSize)
{
    constexpr size_t initialAtlasSize = 160;
    constexpr size_t maxAtlasSize = 4096;
    size_t atlasSize;
    SDL_Surface *atlasSurface = nullptr;
    
    auto chars = getCharSet(face);
    
    for(atlasSize = initialAtlasSize; !atlasSurface && atlasSize < maxAtlasSize; atlasSize += 32) {
        SDL_Log("Trying to create texture atlas of size %d", (int)atlasSize);
        atlasSurface = makeTextureAtlas(face, stroker, chars, atlasSize);
    }
    
    return atlasSurface;
}

SDL_Surface*
FontTextureAtlas::genTextureAtlas(const FontAttributes &attr)
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
    
    FT_Stroker stroker;
    if (FT_Stroker_New(library, &stroker)) {
        throw Exception("Failed to create the font stroker.");
    }
    
    FT_Stroker_Set(stroker, border * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    
    SDL_Surface *atlasSurface = atlasSearch(face, stroker, attr.fontSize);
    
    FT_Stroker_Done(stroker);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    
    if (!atlasSurface) {
        throw Exception("Failed to generate font texture atlas.");
    }
    
    return atlasSurface;
}
