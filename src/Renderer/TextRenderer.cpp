//
//  TextRenderer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/TextRenderer.hpp"
#include "Exception.hpp"
#include "FileUtilities.hpp"

#include <glm/gtc/matrix_transform.hpp> // for glm::ortho

TextRenderer::TextRenderer(std::shared_ptr<spdlog::logger> log,
                           const std::shared_ptr<GraphicsDevice> &dev,
                           const TextAttributes &attributes)
 : _graphicsDevice(dev),
   _windowScaleFactor(1),
   _attributes(attributes),
   _log(log)
{
    _renderPassDescriptor.clear = false;
    
    AttributeFormat attr = {
        4,
        AttributeTypeFloat,
        false,
        sizeof(float) * 4,
        0
    };
    VertexFormat vertexFormat;
    vertexFormat.attributes.push_back(attr);
    _shader = _graphicsDevice->makeShader(vertexFormat,
                                          "text_vert", "text_frag",
                                          true);
    
    regenerateFontTextureAtlas();
    
    TextureSamplerDescriptor samplerDesc = {
        ClampToEdge,
        ClampToEdge,
        Nearest,
        Nearest
    };
    _sampler = _graphicsDevice->makeTextureSampler(samplerDesc);
}

void TextRenderer::draw(const std::shared_ptr<CommandEncoder> &encoder,
                        const glm::ivec4 &vp)
{
    bool projectionValid = false;
    glm::mat4 projection;
    
    encoder->setViewport(vp);
    encoder->setShader(_shader);
    encoder->setFragmentSampler(_sampler, 0);
    encoder->setFragmentTexture(_textureAtlas, 0);
    
    for (auto &string : _strings)
    {
        if (string.viewport != vp) {
            if (!projectionValid) {
                projectionValid = true;
                projection = glm::ortho((float)vp.x, (float)vp.x + vp.z,
                                        (float)vp.y, (float)vp.y + vp.w);
            }
            rebuildUniformBuffer(string, projection);
        }
        
        encoder->setVertexBuffer(string.buffer, 0);
        encoder->setVertexBuffer(string.uniforms, 1);
        encoder->drawPrimitives(Triangles, 0, string.vertexCount, 1);
    }
}

void TextRenderer::rebuildVertexBuffer(String &string)
{
    const std::string &text = string.contents;
    const size_t glyphCount = text.size();
    const size_t bytesPerVertex = sizeof(float) * 4;
    const size_t verticesPerGlyph = 6;
    const size_t vertexCount = verticesPerGlyph * glyphCount;
    const size_t bufferSize = bytesPerVertex * vertexCount;
    
    std::vector<uint8_t> vertexData(bufferSize);
    uint8_t* dst = (uint8_t *)&vertexData[0];
    
    // Iterate through all characters
    glm::vec2 basePos = string.position * (float)_windowScaleFactor;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        auto maybeGlyph = _fontTextureAtlas->getGlyph(*c);
        if (!maybeGlyph) {
            // TODO: Consider drawing a placeholder glyph instead of throwing.
            throw Exception("Unknown character \"{}\"", *c); // TODO: [Exceptions] Throw new exception type for text renderer errors
        }
        
        const auto &glyph = *maybeGlyph;
        
        const glm::vec2 offset(glyph.bearing.x, glyph.bearing.y - glyph.size.y);
        const glm::vec2 pos = basePos + offset;
        const glm::vec2 size(glyph.size.x, glyph.size.y);
        const glm::vec2 &uvo = glyph.uvOrigin;
        const glm::vec2 &uve = glyph.uvExtent;
        
        // Add a quad to the buffer for each glyph.
        float vertexBytes[verticesPerGlyph][4] = {
            { pos.x,          pos.y + size.y,   uvo.x,         uvo.y         },
            { pos.x,          pos.y,            uvo.x,         uvo.y + uve.y },
            { pos.x + size.x, pos.y,            uvo.x + uve.x, uvo.y + uve.y },
            
            { pos.x,          pos.y + size.y,   uvo.x,         uvo.y         },
            { pos.x + size.x, pos.y,            uvo.x + uve.x, uvo.y + uve.y },
            { pos.x + size.x, pos.y + size.y,   uvo.x + uve.x, uvo.y         }
        };
        memcpy(dst, vertexBytes, sizeof(vertexBytes));
        
        // Advance to the next glyph.
        basePos.x += glyph.advance / 64.0f;
        dst += sizeof(vertexBytes);
    }
    
    string.vertexCount = vertexCount;
    string.buffer->replace(std::move(vertexData));
}

void TextRenderer::rebuildUniformBuffer(String &string,
                                        const glm::mat4x4 &projection)
{
    StringUniforms uniforms = {
        string.color,
        projection
    };
    string.uniforms->replace(sizeof(uniforms), &uniforms);
}

TextRenderer::StringHandle TextRenderer::add(const std::string &str,
                                                 const glm::vec2 &position,
                                                 const glm::vec4 &color)
{
    String s = {
        0,
        str,
        position,
        color,
        glm::ivec4(),
        nullptr,
        nullptr
    };
    _strings.push_front(s);
    auto handle = _strings.begin();
    
    const std::string &text = handle->contents;
    const size_t glyphCount = text.size();
    const size_t bytesPerVertex = sizeof(float) * 4;
    const size_t verticesPerGlyph = 6;
    const size_t vertexCount = verticesPerGlyph * glyphCount;
    const size_t bufferSize = bytesPerVertex * vertexCount;
    
    handle->vertexCount = vertexCount;
    handle->buffer = _graphicsDevice->makeBuffer(bufferSize,
                                                 StaticDraw,
                                                 ArrayBuffer);
    rebuildVertexBuffer(*handle);
    
    handle->uniforms = _graphicsDevice->makeBuffer(sizeof(StringUniforms),
                                                   StaticDraw,
                                                   UniformBuffer);
    rebuildUniformBuffer(*handle, glm::mat4x4());
    
    return handle;
}

void TextRenderer::remove(TextRenderer::StringHandle &handle)
{
    _strings.erase(handle);
}

void TextRenderer::replaceContents(StringHandle &handle,
                                   const std::string &contents)
{
    handle->contents = contents;
    rebuildVertexBuffer(*handle);
}

void TextRenderer::setWindowScaleFactor(unsigned windowScaleFactor)
{
    assert(windowScaleFactor > 0);
    _windowScaleFactor = windowScaleFactor;
    regenerateFontTextureAtlas();
    for (auto &s : _strings) {
        rebuildVertexBuffer(s);
    }
}

void TextRenderer::regenerateFontTextureAtlas()
{
    TextAttributes attributes = _attributes;
    attributes.fontSize = _attributes.fontSize * _windowScaleFactor;
    _fontTextureAtlas = std::make_unique<FontTextureAtlas>(_log, getPrefPath(), attributes);
    
    SDL_Surface *surface = _fontTextureAtlas->getSurface();
    
    if (!surface) {
        throw Exception("Failed to get the surface for the font texture atlas");
    }
    
    // Turn the SDL surface into a texture.
    TextureDescriptor texDesc = {
        Texture2D,          // type
        RGBA8,              // format
        (size_t)surface->w, // width
        (size_t)surface->h, // height
        1,                  // depth
        4,                  // unpackAlignment
        false,              // generateMipMaps
    };
    
    _textureAtlas = _graphicsDevice->makeTexture(texDesc, surface->pixels);
}
