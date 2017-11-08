//
//  GlyphRendererRegular.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef GlyphRendererRegular_hpp
#define GlyphRendererRegular_hpp

#include "Renderer/TextAttributes.hpp"
#include "Renderer/GlyphRenderer.hpp"

#include <glm/vec2.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct SDL_Surface;


class GlyphRendererRegular : public GlyphRenderer
{
public:
    ~GlyphRendererRegular() = default;
    
    GlyphRendererRegular(FT_Library &library,
                         FT_Face &face,
                         const TextAttributes &attributes);
    
    // Render the glyph into a surface.
    std::shared_ptr<Glyph> render(FT_ULong charcode) override;
    
private:
    // Blit the specified bitmap into the surface.
    void blitGlyph(SDL_Surface *dst,
                   FT_Bitmap &bitmap,
                   const glm::vec4 &color);
};

#endif /* GlyphRendererRegular_hpp */
