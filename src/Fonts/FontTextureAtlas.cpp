//
//  FontTextureAtlas.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Fonts/FontTextureAtlas.hpp"
#include "Fonts/FontTextureAtlasBuilder.hpp"
#include "Exception.hpp"

#include "SDL.h"
#include "SDL_image.h"

#include <fstream>
#include <cereal/types/unordered_map.hpp>
#include "CerealGLM.hpp"

FontTextureAtlas::~FontTextureAtlas()
{
    SDL_FreeSurface(_atlasSurface);
}

FontTextureAtlas::FontTextureAtlas(std::shared_ptr<spdlog::logger> log,
                                   const boost::filesystem::path &cacheDir,
                                   const TextAttributes &attributes)
 : _log(log)
{
    std::hash<TextAttributes> hasher;
    const std::string baseName = "font" + std::to_string(attributes.fontSize)
                               + "_" + std::to_string(hasher(attributes));
    const boost::filesystem::path atlasImageFilename = cacheDir / (baseName + ".png");
    const boost::filesystem::path atlasDictionaryFilename = cacheDir / (baseName + ".cereal");
    
#ifdef FORCE_REBUILD_FONT_TEXTURE_ATLAS
    constexpr bool forceRebuild = true;
#else
    constexpr bool forceRebuild = false;
#endif
    
    // Font texture atlas is cached between runs of the game.
    if (!forceRebuild &&
        boost::filesystem::exists(atlasImageFilename) &&
        boost::filesystem::exists(atlasDictionaryFilename)) {
        
        _log->info("Loading font texture atlas from files: \"{}\" and \"{}\"",
                   atlasImageFilename.string(),
                   atlasDictionaryFilename.string());
        
        _atlasSurface = IMG_Load(atlasImageFilename.string().c_str());
        
        std::ifstream is(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryInputArchive archive(is);
        archive(_glyphs);
    } else {
        FontTextureAtlasBuilder builder(_log, attributes);
        _atlasSurface = builder.copySurface();
        _glyphs = builder.getGlyphs();
        
        _log->info("Saving font texture atlas to files: \"{}\" and \"{}\"",
                   atlasImageFilename.string(),
                   atlasDictionaryFilename.string());
        
        IMG_SavePNG(_atlasSurface, atlasImageFilename.string().c_str());
        
        std::ofstream os(atlasDictionaryFilename.string().c_str(), std::ios::binary);
        cereal::BinaryOutputArchive archive(os);
        archive(_glyphs);
    }
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
