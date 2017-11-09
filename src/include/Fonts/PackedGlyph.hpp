//
//  PackedGlyph.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef PackedGlyph_hpp
#define PackedGlyph_hpp

#include <glm/vec2.hpp>

// A glyph that's been packed into a texture atlas.
struct PackedGlyph
{
    glm::vec2 uvOrigin;
    glm::vec2 uvExtent;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned advance; // Given in 1/64 points.
    
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(uvOrigin, uvExtent, size, bearing, advance);
    }
};

#endif /* PackedGlyph_hpp */
