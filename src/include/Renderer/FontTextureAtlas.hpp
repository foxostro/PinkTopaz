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
#include "Renderer/TextAttributes.hpp"

#include <glm/vec2.hpp>

#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "SDL.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include "Exception.hpp"

class Glyph
{
public:
    ~Glyph()
    {
        SDL_FreeSurface(_surface);
    }
    
    Glyph(const glm::ivec2 &bearing, unsigned advance, SDL_Surface *surface)
     : _bearing(bearing), _advance(advance), _surface(surface)
    {}
    
    inline const glm::ivec2& getBearing() const { return _bearing; }
    inline unsigned getAdvance() const { return _advance; }
    inline SDL_Surface* getSurface() { return _surface; }
    
    glm::ivec2 getSize()
    {
        return glm::ivec2(_surface->w, _surface->h);
    }
    
    // Blit the glyph into the destination surface at the cursor position.
    void blit(SDL_Surface *dstSurface, const glm::ivec2 &cursor)
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
    
private:
    glm::ivec2 _bearing;
    unsigned _advance;
    SDL_Surface *_surface;
};

class GlyphRenderer
{
public:
    virtual ~GlyphRenderer() = default;
    
    GlyphRenderer(FT_Library &library,
                  FT_Face &face,
                  const TextAttributes &attributes)
     : _library(library), _face(face), _attributes(attributes)
    {}
    
    // Render the glyph into a surface.
    virtual std::unique_ptr<Glyph> render(FT_ULong charcode) = 0;
    
    inline const TextAttributes& getAttributes() const { return _attributes; }
    inline FT_Face& getFace() { return _face; }
    inline FT_Library& getLibrary() { return _library; }
    
private:
    FT_Library &_library;
    FT_Face &_face;
    TextAttributes _attributes;
};

class GlyphRendererOutline : public GlyphRenderer
{
public:
    ~GlyphRendererOutline();
    
    GlyphRendererOutline(FT_Library &library,
                         FT_Face &face,
                         const TextAttributes &attributes);
    
    // Render the glyph into a surface.
    std::unique_ptr<Glyph> render(FT_ULong charcode) override;
    
private:
    enum GlyphStyle
    {
        BORDER,
        INTERIOR
    };
    
    enum BlitMode
    {
        COLOR_KEY,
        BLEND
    };
    
    // Blit the specified bitmap into the surface at the cursor position.
    void blitGlyph(SDL_Surface *dst,
                   FT_Bitmap &bitmap,
                   const glm::ivec2 &cursor,
                   const glm::vec4 &color,
                   BlitMode mode);
    
    // Gets the glyph for the specified character.
    FT_Glyph getGlyph(FT_ULong charcode, GlyphStyle style);
    
    FT_Stroker _stroker;
};

// Creates a texture atlas which contains packed font glyphs for string drawing.
// Besides generating the image itself, this class also records metadata on how
// to access the atlas such as which texture coordinates correspond to which
// characters.
class FontTextureAtlas
{
public:
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
    
    ~FontTextureAtlas() = default;
    
    FontTextureAtlas(GraphicsDevice &graphicsDevice,
                     const TextAttributes &attributes);
    
    inline std::shared_ptr<Texture> getTexture()
    {
        return _textureAtlas;
    }
    
    inline boost::optional<PackedGlyph> getGlyph(char c) const
    {
        auto iter = _glyphs.find(c);
        
        if (iter != _glyphs.end()) {
            return boost::make_optional(iter->second);
        } else {
            return boost::none;
        }
    }
    
private:
    // Draw a glyph into the specified surface.
    bool placeGlyph(FT_Face &face,
                    GlyphRenderer &glyphRenderer,
                    FT_ULong charcode,
                    SDL_Surface *atlasSurface,
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
                     GlyphRenderer &glyphRenderer,
                     const std::vector<std::pair<char, unsigned>> &chars,
                     size_t atlasSize);
    
    // Searches for, and returns, the smallest font texture atlas that can
    // accomodate the specified font at the specified font size.
    // When this method returns, `_glyphs' will contain valid glyph metrics.
    SDL_Surface* atlasSearch(FT_Face &face, GlyphRenderer &glyphRenderer);
    
    // Returns a font texture atlas for the specified text attributes.
    SDL_Surface* genTextureAtlas(const TextAttributes &attr);
    
    std::shared_ptr<Texture> _textureAtlas;
    std::unordered_map<char, PackedGlyph> _glyphs;
};

#endif /* FontTextureAtlas_hpp */
