//
//  TextRenderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef TextRenderer_hpp
#define TextRenderer_hpp

#include "Fonts/FontTextureAtlas.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Buffer.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <memory>

// Draws strings on screen.
// All strings must be in one font face and one font size.
class TextRenderer
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
    
    TextRenderer(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const TextAttributes &attributes);
    
    ~TextRenderer() = default;
    
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
    // Rebuilds the internal vertex buffer for a string. This is useful
    // when string contents have changed, for example.
    void rebuildVertexBuffer(String &string);
    
    // Rebuilds the internal uniform buffer for a string. This is useful
    // when the projection changes, or the color changes, for example.
    void rebuildUniformBuffer(String &string, const glm::mat4x4 &proj);
    
    // Regenerate the texture atlas according to the current window scale.
    void regenerateFontTextureAtlas();
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::unique_ptr<FontTextureAtlas> _fontTextureAtlas;
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<TextureSampler> _sampler;
    std::shared_ptr<Texture> _textureAtlas;
    VertexFormat _vertexFormat;
    RenderPassDescriptor _renderPassDescriptor;
    std::list<String> _strings;
    unsigned _windowScaleFactor;
    TextAttributes _attributes;
};

#endif /* TextRenderer_hpp */
