//
//  TextureArrayLoader.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/1/18.
//
//

#include "Renderer/TextureArrayLoader.hpp"
#include "Exception.hpp"
#include "SDL_image.h"

TextureArrayLoader::TextureArrayLoader(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
 : _graphicsDevice(graphicsDevice)
{}

std::shared_ptr<Texture> TextureArrayLoader::load(const boost::filesystem::path &imageFileName)
{
    SDL_Surface *surface = IMG_Load(imageFileName.c_str());
    
    if (!surface) {
        throw Exception("Failed to load texture array from file: %s", imageFileName.c_str());
    }
    
    TextureDescriptor texDesc = {
        /* .type    = */ Texture2DArray,
        /* .format  = */ BGRA8,
        /* .width   = */ static_cast<size_t>(surface->w),
        /* .height  = */ static_cast<size_t>(surface->w),
        /* .depth   = */ static_cast<size_t>(surface->h / surface->w),
        /* .unpackAlignment = */ 4,
        /* .generateMipMaps = */ true,
    };
    auto texture = _graphicsDevice->makeTexture(texDesc, surface->pixels);
    
    SDL_FreeSurface(surface);
    
    return texture;
}
