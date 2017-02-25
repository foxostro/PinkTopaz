//
//  TextureArrayOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#include "Renderer/OpenGL/TextureArrayOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "SDL.h"
#include "SDL_image.h"

namespace PinkTopaz::Renderer::OpenGL {
    
    TextureArrayOpenGL::TextureArrayOpenGL(CommandQueue &commandQueue,
                                           const char *fileName)
     : _handle(0),
       _commandQueue(commandQueue)
    {
        _commandQueue.enqueue([=]{
            SDL_Surface *surface = IMG_Load(fileName);
            
            // Assume square tiles arranged vertically.
            const int tileWidth = surface->w;
            const int tileHeight = surface->w;
            const int numTiles = surface->h / surface->w;
            
            GLuint handle = 0;
            glGenTextures(1, &handle);
            glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
            
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
            
            SDL_FreeSurface(surface);

            this->_handle = handle;
            
            CHECK_GL_ERROR();
        });
    }
    
    TextureArrayOpenGL::~TextureArrayOpenGL()
    {
        GLuint handle = _handle;
        _commandQueue.enqueue([handle]{
            glDeleteTextures(1, &handle);
            CHECK_GL_ERROR();
        });
    }

} // namespace PinkTopaz::Renderer::OpenGL
