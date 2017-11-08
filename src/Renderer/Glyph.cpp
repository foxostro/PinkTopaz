//
//  Glyph.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#include "Renderer/Glyph.hpp"
#include "Exception.hpp"
#include "SDL.h"

Glyph::~Glyph()
{
    SDL_FreeSurface(_surface);
}

Glyph::Glyph(const glm::ivec2 &bearing, unsigned advance, SDL_Surface *surface)
 : _bearing(bearing), _advance(advance), _surface(surface)
{}

glm::ivec2 Glyph::getSize()
{
    return glm::ivec2(_surface->w, _surface->h);
}

// Blit the glyph into the destination surface at the cursor position.
void Glyph::blit(SDL_Surface *dstSurface, const glm::ivec2 &cursor)
{
    SDL_Rect src = {
        0, 0,
        getSize().x, getSize().y
    };
    
    SDL_Rect dst = {
        cursor.x, cursor.y,
        getSize().x, getSize().y
    };
    
    if (SDL_BlitSurface(getSurface(), &src, dstSurface, &dst)) {
        throw Exception("Failed to blit glpyh into destination surface.");
    }
}
