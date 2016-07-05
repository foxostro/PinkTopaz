//
//  TextureArray.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#include "TextureArray.hpp"
#include "glUtilities.hpp"
#include "SDL.h"
#include "SDL_image.h"

namespace PinkTopaz {
    
    TextureArray::TextureArray(const char *filePath) : _handle(0)
    {
        SDL_Surface *surface = IMG_Load("terrain.png");
        
        // Assume square tiles arranged vertically.
        const int tileWidth = surface->w;
        const int tileHeight = surface->w;
        const int numTiles = surface->h / surface->w;

        glGenTextures(1, &_handle);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _handle);
        
        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
                     tileWidth, tileHeight, numTiles,
                     0, GL_BGRA, GL_UNSIGNED_BYTE,
                     surface->pixels);
        
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        CHECK_GL_ERROR();
        
        SDL_FreeSurface(surface);
    }
    
    TextureArray::~TextureArray()
    {
        glDeleteTextures(1, &_handle);
    }
    
    void TextureArray::bind()
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, _handle);
    }

} // namespace PinkTopaz
