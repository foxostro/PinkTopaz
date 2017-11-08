//
//  FontTextureAtlas.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/FontTextureAtlas.hpp"
#include "Renderer/FontTextureAtlasBuilder.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include "SDL.h"
#include "SDL_image.h"

#include <fstream>
#include <cereal/types/unordered_map.hpp>
#include "CerealGLM.hpp"

static constexpr bool FORCE_REBUILD = true;

FontTextureAtlas::FontTextureAtlas(GraphicsDevice &graphicsDevice,
                                   const TextAttributes &attributes)
{
    const std::string baseName = "font" + std::to_string(attributes.fontSize);
    const boost::filesystem::path prefPath = getPrefPath();
    const boost::filesystem::path atlasImageFilename = prefPath / (baseName + ".png");
    const boost::filesystem::path atlasDictionaryFilename = prefPath / (baseName + ".cereal");
    
    SDL_Surface *atlasSurface = nullptr;
    
    // Font texture atlas is cached between runs of the game.
    if (!FORCE_REBUILD &&
        boost::filesystem::exists(atlasImageFilename) &&
        boost::filesystem::exists(atlasDictionaryFilename)) {
        
        SDL_Log("Loading font texture atlas from files: \"%s\" and \"%s\"",
                atlasImageFilename.string().c_str(),
                atlasDictionaryFilename.string().c_str());
        
        atlasSurface = IMG_Load(atlasImageFilename.string().c_str());
        
        std::ifstream is(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryInputArchive archive(is);
        archive(_glyphs);
    } else {
        FontTextureAtlasBuilder builder(attributes);
        atlasSurface = builder.copySurface();
        _glyphs = builder.getGlyphs();
        
        SDL_Log("Saving font texture atlas to files: \"%s\" and \"%s\"",
                atlasImageFilename.string().c_str(),
                atlasDictionaryFilename.string().c_str());
        
        IMG_SavePNG(atlasSurface, atlasImageFilename.string().c_str());
        
        std::ofstream os(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryOutputArchive archive(os);
        archive(_glyphs);
    }
    
    // Turn the SDL surface into a texture.
    TextureDescriptor texDesc = {
        Texture2D,                            // type
        RGBA8,                                // format
        static_cast<size_t>(atlasSurface->w), // width
        static_cast<size_t>(atlasSurface->h), // height
        1,                                    // depth
        4,                                    // unpackAlignment
        false,                                // generateMipMaps
    };
    _textureAtlas = graphicsDevice.makeTexture(texDesc, atlasSurface->pixels);
    SDL_FreeSurface(atlasSurface);
}

boost::optional<PackedGlyph> FontTextureAtlas::getGlyph(char c) const
{
    auto iter = _glyphs.find(c);
    
    if (iter != _glyphs.end()) {
        return boost::make_optional(iter->second);
    } else {
        return boost::none;
    }
}
