//
//  GlyphRendererOutline.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef GlyphRendererOutline_hpp
#define GlyphRendererOutline_hpp

#include "Renderer/TextAttributes.hpp"
#include "Renderer/GlyphRenderer.hpp"

#include <glm/vec2.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

struct SDL_Surface;


class GlyphRendererOutline : public GlyphRenderer
{
public:
    ~GlyphRendererOutline();
    
    GlyphRendererOutline(FT_Library &library,
                         FT_Face &face,
                         const TextAttributes &attributes);
    
    // Render the glyph into a surface.
    std::unique_ptr<Glyph> render(FT_ULong charcode) override;
    
private:
    enum GlyphStyle
    {
        BORDER,
        INTERIOR
    };
    
    enum BlitMode
    {
        COLOR_KEY,
        BLEND
    };
    
    // Blit the specified bitmap into the surface at the cursor position.
    void blitGlyph(SDL_Surface *dst,
                   FT_Bitmap &bitmap,
                   const glm::ivec2 &cursor,
                   const glm::vec4 &color,
                   BlitMode mode);
    
    // Gets the glyph for the specified character.
    FT_Glyph getGlyph(FT_ULong charcode, GlyphStyle style);
    
    FT_Stroker _stroker;
};

#endif /* GlyphRendererOutline_hpp */
