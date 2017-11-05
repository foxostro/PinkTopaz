//
//  StringRenderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef StringRenderer_hpp
#define StringRenderer_hpp

#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "CerealGLM.hpp"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <unordered_map>
#include <list>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SDL.h"
#include <boost/filesystem.hpp>

class StringRenderer
{
public:
    struct StringUniforms
    {
        glm::vec4 color;
        glm::mat4 projection;
    };
    
    struct String
    {
        size_t vertexCount;
        std::string contents;
        glm::vec2 position;
        glm::vec4 color;
        glm::ivec4 viewport;
        std::shared_ptr<Buffer> buffer;
        std::shared_ptr<Buffer> uniforms;
    };
    
    using StringHandle = std::list<String>::iterator;
    
    StringRenderer(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                   const boost::filesystem::path &fontName,
                   unsigned fontSize);
    ~StringRenderer() = default;
    
    // Draws all registered strings.
    void draw(const std::shared_ptr<CommandEncoder> &encoder,
              const glm::ivec4 &viewport);
    
    // Add a string to be rendered.
    StringHandle add(const std::string &contents,
                     const glm::vec2 &position,
                     const glm::vec4 &color);
    
    // Remove a string that was previously added.
    void remove(StringHandle &handle);
    
    // Change the contents of a string that was previously added.
    void replaceContents(StringHandle &handle, const std::string &contents);
    
    // Set the scale factor for converting from points to pixels.
    // This is necessary on macOS systems with Retina displays, where the scale
    // factor is 2. Other systems usually have a scale factor of 1, which is the
    // default.
    void setWindowScaleFactor(unsigned windowScaleFactor);
    
private:
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
    
    // Gets the image bytes from the specified surface. The image data
    // comes from only the RED components of the specified surface. All
    // other components are discarded.
    static std::vector<uint8_t> getGrayScaleImageBytes(SDL_Surface *surf);
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
    
    // Returns a texture which holds the font texture atlas for the
    // specified font and specified font size.
    std::shared_ptr<Texture> makeTextureAtlas(const boost::filesystem::path &fontName,
                                              unsigned fontSize);
    
    // Rebuilds the internal vertex buffer for a string. This is useful
    // when string contents have changed, for example.
    void rebuildVertexBuffer(String &string);
    
    // Rebuilds the internal uniform buffer for a string. This is useful
    // when the projection changes, or the color changes, for example.
    void rebuildUniformBuffer(String &string, const glm::mat4x4 &proj);
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<Texture> _textureAtlas;
    std::unordered_map<char, Glyph> _glyphs;
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<TextureSampler> _sampler;
    VertexFormat _vertexFormat;
    RenderPassDescriptor _renderPassDescriptor;
    std::list<String> _strings;
    unsigned _windowScaleFactor;
    boost::filesystem::path _fontName;
    unsigned _fontSize;
};

#endif /* StringRenderer_hpp */
