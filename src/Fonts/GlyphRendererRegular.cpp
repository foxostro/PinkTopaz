//
//  GlyphRendererRegular.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#include "Fonts/GlyphRendererRegular.hpp"
#include "Exception.hpp"

#include "SDL.h"

GlyphRendererRegular::GlyphRendererRegular(FT_Library &library,
                                           FT_Face &face,
                                           const TextAttributes &attributes)
 : GlyphRenderer(library, face, attributes)
{
    if(attributes.border == 0) {
        throw Exception("Text border must be zero for GlyphRendererRegular.");
    }
}

std::shared_ptr<Glyph>
GlyphRendererRegular::render(FT_ULong charcode)
{
    FT_Face &face = getFace();
    
    if (FT_Load_Char(face, charcode, FT_LOAD_RENDER)) {
        throw Exception("Failed to load the glyph \"%c\"", (char)charcode);
    }
    
    FT_Bitmap &bitmap = face->glyph->bitmap;
    
    // Create a surface to hold the final glyph image.
    SDL_Surface *surface = SDL_CreateRGBSurface(0,
                                                bitmap.width,
                                                bitmap.rows,
                                                (int)(sizeof(uint32_t)*8),
                                                0x000000ff,
                                                0x0000ff00,
                                                0x00ff0000,
                                                0xff000000);
    
    if (!surface) {
        throw Exception("Failed to create surface for glyph: \"%c\"", charcode);
    }
    
    blitGlyph(surface, bitmap, getAttributes().color);
    
    const glm::ivec2 bearing(face->glyph->bitmap_left, face->glyph->bitmap_top);
    const unsigned advance = face->glyph->advance.x;
    return std::make_shared<Glyph>((char)charcode, bearing, advance, surface);
}

void GlyphRendererRegular::blitGlyph(SDL_Surface *dstSurface,
                                     FT_Bitmap &bitmap,
                                     const glm::vec4 &color)
{
    assert(dstSurface);
    
    const size_t n = bitmap.width * bitmap.rows;
    const uint8_t *glyphBytes = bitmap.buffer;
    
    // Convert grayscale image to RGBA so we can use SDL_Surface.
    uint32_t *pixels = (uint32_t *)dstSurface->pixels;
    const uint8_t rshift = dstSurface->format->Bshift;
    const uint8_t gshift = dstSurface->format->Gshift;
    const uint8_t bshift = dstSurface->format->Rshift;
    const uint8_t ashift = dstSurface->format->Ashift;
    
    for (size_t i = 0; i < n; ++i) {
        const float luminance = (float)glyphBytes[i] / 255.f;
        const uint32_t redComp   = color.r * luminance * 255.f;
        const uint32_t greenComp = color.g * luminance * 255.f;
        const uint32_t blueComp  = color.b * luminance * 255.f;
        const uint32_t alphaComp = color.a * luminance * 255.f;
        const uint32_t pixel = (redComp   << rshift)
                             | (greenComp << gshift)
                             | (blueComp  << bshift)
                             | (alphaComp << ashift);
        pixels[i] = pixel;
    }
}
