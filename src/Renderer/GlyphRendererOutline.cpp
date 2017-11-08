//
//  GlyphRendererOutline.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#include "Renderer/GlyphRendererOutline.hpp"
#include "Exception.hpp"

#include "SDL.h"

GlyphRendererOutline::~GlyphRendererOutline()
{
    FT_Stroker_Done(_stroker);
}

GlyphRendererOutline::GlyphRendererOutline(FT_Library &library,
                                           FT_Face &face,
                                           const TextAttributes &attributes)
 : GlyphRenderer(library, face, attributes)
{
    if(attributes.border <= 0 || attributes.border > 12) {
        throw Exception("Text border failed sanity check: %d", attributes.border);
    }
    
    if (FT_Stroker_New(library, &_stroker)) {
        throw Exception("Failed to create the font stroker.");
    }
    
    FT_Stroker_Set(_stroker,
                   attributes.border * 64,
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);
}

std::shared_ptr<Glyph>
GlyphRendererOutline::render(FT_ULong charcode)
{
    FT_Glyph glyphBorder = getGlyph(charcode, BORDER);
    FT_Glyph_To_Bitmap(&glyphBorder, FT_RENDER_MODE_NORMAL, nullptr, true);
    assert(glyphBorder->format == FT_GLYPH_FORMAT_BITMAP);
    FT_BitmapGlyph bitmapGlyphBorder = (FT_BitmapGlyph)glyphBorder;
    FT_Bitmap &bitmapBorder = bitmapGlyphBorder->bitmap;
    
    FT_Glyph glyphInterior = getGlyph(charcode, INTERIOR);
    FT_Glyph_To_Bitmap(&glyphInterior, FT_RENDER_MODE_NORMAL, nullptr, true);
    assert(glyphInterior->format == FT_GLYPH_FORMAT_BITMAP);
    FT_BitmapGlyph bitmapGlyphInterior = (FT_BitmapGlyph)glyphInterior;
    
    // Create a surface to hold the final glyph image.
    SDL_Surface *finalGlyphSurface = SDL_CreateRGBSurface(0,
                                                          bitmapBorder.width,
                                                          bitmapBorder.rows,
                                                          (int)(sizeof(uint32_t)*8),
                                                          0x000000ff,
                                                          0x0000ff00,
                                                          0x00ff0000,
                                                          0xff000000);
    
    if (!finalGlyphSurface) {
        throw Exception("Failed to create surface for glyph: \"%c\"", charcode);
    }
    
    if (SDL_SetSurfaceBlendMode(finalGlyphSurface, SDL_BLENDMODE_BLEND)) {
        throw Exception("Failed to set blend mode for glyph surface: \"%c\"", charcode);
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
    
    blitGlyph(finalGlyphSurface,
              bitmapGlyphInterior->bitmap,
              offset,
              getAttributes().color,
              COLOR_KEY);
    
    blitGlyph(finalGlyphSurface,
              bitmapGlyphBorder->bitmap,
              glm::ivec2(0, 0),
              getAttributes().borderColor,
              BLEND);
    
    FT_Done_Glyph(glyphInterior);
    FT_Done_Glyph(glyphBorder);
    
    const glm::ivec2 bearing(bitmapGlyphBorder->left, bitmapGlyphBorder->top);
    const unsigned advance = getFace()->glyph->advance.x;
    return std::make_shared<Glyph>((char)charcode,
                                   bearing,
                                   advance,
                                   finalGlyphSurface);
}

void GlyphRendererOutline::blitGlyph(SDL_Surface *dstSurface,
                                     FT_Bitmap &bitmap,
                                     const glm::ivec2 &cursor,
                                     const glm::vec4 &color,
                                     BlitMode mode)
{
    assert(dstSurface);
    
    const size_t width = bitmap.width;
    const size_t height = bitmap.rows;
    const size_t n = width * height;
    const uint8_t *glyphBytes = bitmap.buffer;
    
    // Convert grayscale image to RGBA so we can use SDL_Surface.
    std::vector<uint32_t> pixels(n);
    const uint8_t rshift = dstSurface->format->Bshift;
    const uint8_t gshift = dstSurface->format->Gshift;
    const uint8_t bshift = dstSurface->format->Rshift;
    const uint8_t ashift = dstSurface->format->Ashift;
    
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
    SDL_Rect srcRect = {
        0, 0,
        (int)width, (int)height,
    };
    
    SDL_Rect dstRect = {
        cursor.x, cursor.y,
        (int)width, (int)height,
    };
    
    if (SDL_BlitSurface(glyphSurface, &srcRect, dstSurface, &dstRect)) {
        throw Exception("Failed to blit glpyh into font texture atlas surface.");
    }
    
    SDL_FreeSurface(glyphSurface);
}

FT_Glyph GlyphRendererOutline::getGlyph(FT_ULong charcode, GlyphStyle style)
{
    FT_Face &face = getFace();
    
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
            if (FT_Glyph_StrokeBorder(&glyph, _stroker, true, true)) {
                FT_Done_Glyph(glyph);
                throw Exception("Failed to stroke the inside of the glyph \"%c\"", (char)charcode);
            }
            break;
            
        case BORDER:
            if (FT_Glyph_Stroke(&glyph, _stroker, true)) {
                FT_Done_Glyph(glyph);
                throw Exception("Failed to stroke the border of the glyph \"%c\"", (char)charcode);
            }
            break;
            
        default:
            assert(!"not reachable");
    }
    
    return glyph;
}
