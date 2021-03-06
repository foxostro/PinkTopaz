//
//  FontTextureAtlas.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlas_hpp
#define FontTextureAtlas_hpp

#include "Fonts/TextAttributes.hpp"
#include "Fonts/PackedGlyph.hpp"

#include <glm/vec2.hpp>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

struct SDL_Surface;

// A font texture atlas appropriate for specified TextAttributes.
// Font texture atlases are cached on disk to avoid regenerating every time.
class FontTextureAtlas
{
public:
    ~FontTextureAtlas();
    
    FontTextureAtlas(std::shared_ptr<spdlog::logger> log,
                     const boost::filesystem::path &cacheDir,
                     const TextAttributes &attributes);
    
    // Get the glyph for the specified character code.
    boost::optional<PackedGlyph> getGlyph(char c) const;
    
    // Get the texture atlas.
    inline SDL_Surface* getSurface()
    {
        return _atlasSurface;
    }
    
private:
    SDL_Surface *_atlasSurface;
    std::unordered_map<char, PackedGlyph> _glyphs;
    std::shared_ptr<spdlog::logger> _log;
};

#endif /* FontTextureAtlas_hpp */
