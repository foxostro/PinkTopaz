//
//  Glyph.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef Glyph_hpp
#define Glyph_hpp

#include <glm/vec2.hpp>
struct SDL_Surface;

class Glyph
{
public:
    ~Glyph();
    Glyph(char charcode,
          const glm::ivec2 &bearing,
          unsigned advance,
          SDL_Surface *surface);
    
    inline char getCharCode() const { return _charcode; }
    inline const glm::ivec2& getBearing() const { return _bearing; }
    inline unsigned getAdvance() const { return _advance; }
    inline SDL_Surface* getSurface() { return _surface; }
    
    glm::ivec2 getSize();
    
    // Blit the glyph into the destination surface at the cursor position.
    void blit(SDL_Surface *dstSurface, const glm::ivec2 &cursor);
    
private:
    char _charcode;
    glm::ivec2 _bearing;
    unsigned _advance;
    SDL_Surface *_surface;
};

#endif /* Glyph_hpp */
