//
//  GlyphRenderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef GlyphRenderer_hpp
#define GlyphRenderer_hpp

#include "Renderer/Glyph.hpp"
#include "Renderer/TextAttributes.hpp"

#include "SDL.h"
#include <glm/vec2.hpp>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

class GlyphRenderer
{
public:
    virtual ~GlyphRenderer() = default;
    
    GlyphRenderer(FT_Library &library,
                  FT_Face &face,
                  const TextAttributes &attributes)
     : _library(library), _face(face), _attributes(attributes)
    {}
    
    // Render the glyph into a surface.
    virtual std::unique_ptr<Glyph> render(FT_ULong charcode) = 0;
    
    inline const TextAttributes& getAttributes() const { return _attributes; }
    inline FT_Face& getFace() { return _face; }
    inline FT_Library& getLibrary() { return _library; }
    
private:
    FT_Library &_library;
    FT_Face &_face;
    TextAttributes _attributes;
};

#endif /* GlyphRenderer_hpp */
