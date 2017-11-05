//
//  FontTextureAtlas.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlas_hpp
#define FontTextureAtlas_hpp

#include "Renderer/Texture.hpp"
#include "Renderer/GraphicsDevice.hpp"

#include <glm/vec2.hpp>

#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SDL.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

// Creates a texture atlas which contains packed font glyphs for string drawing.
// Besides generating the image itself, this class also records metadata on how
// to access the atlas such as which texture coordinates correspond to which
// characters.
class FontTextureAtlas
{
public:
    struct Glyph
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
    
    ~FontTextureAtlas() = default;
    
    FontTextureAtlas(GraphicsDevice &graphicsDevice,
                     const boost::filesystem::path &fontName,
                     unsigned fontSize);
    
    inline std::shared_ptr<Texture> getTexture()
    {
        return _textureAtlas;
    }
    
    inline boost::optional<Glyph> getGlyph(char c) const
    {
        auto iter = _glyphs.find(c);
        
        if (iter != _glyphs.end()) {
            return boost::make_optional(iter->second);
        } else {
            return boost::none;
        }
    }
    
private:
    // Gets the image bytes from the specified surface. The image data
    // comes from only the RED components of the specified surface. All
    // other components are discarded.
    static std::vector<uint8_t> getGrayScaleImageBytes(SDL_Surface *surf);
    
    // Draw a glyph into the specified surface.
    static bool placeGlyph(FT_Face &face,
                           FT_ULong c,
                           SDL_Surface *atlasSurface,
                           std::unordered_map<char, Glyph> &glyphs,
                           glm::ivec2 &cursor,
                           size_t &rowHeight);
    
    // Returns a sorted list of pairs where each pair is made of a character
    // that belongs in the font texture atlas, and it's height.
    std::vector<std::pair<char, unsigned>> getCharSet(FT_Face &f);
    
    // Creates a font texture atlas with the specified character set.
    // The atlas size is directly specified. Though, this method will return
    // false if it is not possible to pack all characters into a surface of
    // this size.
    SDL_Surface*
    makeTextureAtlas(FT_Face &face,
                     const std::vector<std::pair<char, unsigned>> &chars,
                     size_t atlasSize);
    
    // Searches for, and returns, the smallest font texture atlas that can
    // accomodate the specified font at the specified font size.
    // When this method returns, `_glyphs' will contain valid glyph metrics.
    SDL_Surface* atlasSearch(FT_Face &face, unsigned fontSize);
    
    // Returns a font texture atlas for the specified font and size.
    SDL_Surface* genTextureAtlas(const boost::filesystem::path &fontName,
                                 unsigned fontSize);
    
    std::shared_ptr<Texture> _textureAtlas;
    std::unordered_map<char, Glyph> _glyphs;
};

#endif /* FontTextureAtlas_hpp */
