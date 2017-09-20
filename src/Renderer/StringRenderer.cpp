//
//  StringRenderer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/StringRenderer.hpp"
#include "Exception.hpp"

#include <glm/gtc/matrix_transform.hpp> // for glm::ortho

#include "SDL.h"
#include "SDL_image.h"

static boost::filesystem::path getPrefPath()
{
    char *s = SDL_GetPrefPath("foxostro", "PinkTopaz");
    boost::filesystem::path prefPath(s);
    SDL_free(s);
    return prefPath;
}

std::vector<uint8_t> StringRenderer::getGrayScaleImageBytes(SDL_Surface *surface)
{
    const size_t w = surface->w;
    const size_t h = surface->h;
    const uint32_t mask = surface->format->Rmask;
    const uint8_t shift = surface->format->Rshift;
    
    // We only want to store the RED components in the GPU texture.
    std::vector<uint8_t> atlasPixels(w * h);
    
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }
    
    uint32_t *srcRow = (uint32_t *)surface->pixels;
    uint8_t *dstRow = (uint8_t *)&atlasPixels[0];
    for(size_t y = 0; y < h; ++y)
    {
        for(size_t x = 0; x < w; ++x)
        {
            uint32_t pixel = srcRow[x];
            uint8_t component = (pixel & mask) >> shift;
            dstRow[x] = component;
        }
        
        srcRow += surface->pitch / surface->format->BytesPerPixel;
        dstRow += w;
    }
    
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
    
    return atlasPixels;
}

bool StringRenderer::placeGlyph(FT_Face &face,
                                FT_ULong c,
                                SDL_Surface *atlasSurface,
                                std::map<char, Glyph> &glyphs,
                                glm::ivec2 &cursor,
                                size_t &rowHeight)
{
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        throw Exception("Failed to load the glyph %c.", (char)c);
    }
    
    const size_t width = face->glyph->bitmap.width;
    const size_t height = face->glyph->bitmap.rows;
    const size_t n = width * height;
    const uint8_t *glyphBytes = face->glyph->bitmap.buffer;
    
    rowHeight = std::max(rowHeight, height);
    
    // Validate the cursor. Can the glyph fit on this row?
    if ((cursor.x + width) >= atlasSurface->w) {
        // Go to the next row.
        cursor.x = 0;
        cursor.y += rowHeight;
        rowHeight = height;
        
        // Have we run out of rows? If so then try a bigger atlas.
        if ((cursor.y + height) >= atlasSurface->h) {
            return false;
        }
    }
    
    // Convert grayscale image to RGBA so we can use SDL_Surface.
    std::vector<uint32_t> pixels(n);
    const uint8_t rshift = atlasSurface->format->Rshift;
    const uint8_t gshift = atlasSurface->format->Gshift;
    const uint8_t bshift = atlasSurface->format->Bshift;
    const uint8_t ashift = atlasSurface->format->Ashift;
    for(size_t i = 0; i < n; ++i)
    {
        uint32_t component = glyphBytes[i];
        constexpr uint8_t alpha = 0xff;
        pixels[i] = (alpha << ashift)
        | (component << rshift)
        | (component << gshift)
        | (component << bshift);
    }
    
    // Create a surface with the glpyh image.
    SDL_Surface *glyphSurface = SDL_CreateRGBSurfaceFrom(&pixels[0],
                                                         width,
                                                         height,
                                                         sizeof(uint32_t) * 8,
                                                         width * sizeof(uint32_t),
                                                         0x000000ff,
                                                         0x0000ff00,
                                                         0x00ff0000,
                                                         0xff000000);
    
    // Blit the glyph into the texture atlas at the cursor position.
    SDL_Rect src = {
        0, 0,
        (int)width, (int)height,
    };
    
    SDL_Rect dst = {
        cursor.x, cursor.y,
        (int)width, (int)height,
    };
    
    SDL_BlitSurface(glyphSurface, &src, atlasSurface, &dst);
    SDL_FreeSurface(glyphSurface);
    
    // Now store the glyph for later use.
    Glyph glyph = {
        glm::vec2((float)cursor.x / atlasSurface->w,
                  (float)cursor.y / atlasSurface->w),
        glm::vec2((float)width / atlasSurface->h,
                  (float)height / atlasSurface->h),
        glm::ivec2(width, height),
        glm::ivec2(face->glyph->bitmap_left,
                   face->glyph->bitmap_top),
        (unsigned)face->glyph->advance.x
    };
    glyphs.insert(std::pair<char, Glyph>((char)c, glyph));
    
    // Increment the cursor. We've already validated for this glyph.
    cursor.x += width;
    
    return true;
}

SDL_Surface*
StringRenderer::makeTextureAtlas(FT_Face &face,
                                 const std::vector<std::pair<char, unsigned>> &characters,
                                 size_t size)
{
    _glyphs.clear();
    
    size_t rowHeight = 0;
    glm::ivec2 cursor(0, 0);
    
    SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0,
                                                     size, size,
                                                     sizeof(uint32_t) * 8,
                                                     0x000000ff,
                                                     0x0000ff00,
                                                     0x00ff0000,
                                                     0xff000000);
    
    for (auto &character : characters)
    {
        char c = character.first;
        if (!placeGlyph(face, c, atlasSurface, _glyphs,
                        cursor, rowHeight)) {
            SDL_FreeSurface(atlasSurface);
            atlasSurface = nullptr;
            _glyphs.clear();
            break;
        }
    }
    
    return atlasSurface;
}

std::vector<std::pair<char, unsigned>>
StringRenderer::getCharSet(FT_Face &face)
{
    std::vector<std::pair<char, unsigned>> characters;
    
    for (FT_ULong c = 32; c < 127; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            throw Exception("Failed to load the glyph %c.", (char)c);
        }
        
        unsigned height = face->glyph->bitmap.rows;
        characters.emplace_back(std::make_pair((char)c, height));
    }
    
    std::sort(characters.begin(), characters.end(),
              [](const std::pair<char, int> &left,
                 const std::pair<char, int> &right) -> bool {
                  return (left.second < right.second);
              });
    
    return characters;
}

SDL_Surface* StringRenderer::atlasSearch(FT_Face &face, unsigned fontSize)
{
    constexpr size_t initialAtlasSize = 288;
    constexpr size_t maxAtlasSize = 4096;
    size_t atlasSize;
    SDL_Surface *atlasSurface = nullptr;
    
    auto chars = getCharSet(face);
    
    for(atlasSize = initialAtlasSize; !atlasSurface && atlasSize < maxAtlasSize; atlasSize += 32)
    {
        SDL_Log("Trying to create texture atlas of size %d", (int)atlasSize);
        atlasSurface = makeTextureAtlas(face, chars, atlasSize);
    }
    
    return atlasSurface;
}

SDL_Surface* StringRenderer::genTextureAtlas(const boost::filesystem::path &fontName,
                                             unsigned fontSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw Exception("Failed to initialize Freetype.");
    }
    
    FT_Face face;
    if (FT_New_Face(ft, fontName.string().c_str(), 0, &face)) {
        throw Exception("Failed to load the font: %s", fontName.c_str());
    }
    
    if (FT_Set_Pixel_Sizes(face, 0, fontSize)) {
        throw Exception("Failed to set the font size.");
    }
    
    SDL_Surface *atlasSurface = atlasSearch(face, fontSize);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    if (!atlasSurface) {
        throw Exception("Failed to generate font texture atlas.");
    }
    
    return atlasSurface;
}

std::shared_ptr<Texture>
StringRenderer::makeTextureAtlas(const boost::filesystem::path &fontName,
                                 unsigned fontSize)
{
    // Font texture atlas is cached between runs of the game.
    boost::filesystem::path atlasFileName = getPrefPath();
    atlasFileName.append("font" + std::to_string(fontSize) + ".png");
    
    SDL_Surface *atlasSurface = genTextureAtlas(fontName, fontSize);
    IMG_SavePNG(atlasSurface, atlasFileName.string().c_str());
    SDL_Log("Saving font texture atlas to file: %s", atlasFileName.c_str());
    
    // We only want to store the RED components in the GPU texture.
    std::vector<uint8_t> atlasPixels = getGrayScaleImageBytes(atlasSurface);
    TextureDescriptor texDesc = {
        Texture2D,
        R8,
        (size_t)atlasSurface->w,
        (size_t)atlasSurface->h,
        1,
        1,
        false
    };
    SDL_FreeSurface(atlasSurface);
    
    return _graphicsDevice->makeTexture(texDesc, atlasPixels);
}

StringRenderer::StringRenderer(const std::shared_ptr<GraphicsDevice> &dev,
                               const boost::filesystem::path &fontName,
                               unsigned fontSize)
: _graphicsDevice(dev)
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
    
    _textureAtlas = makeTextureAtlas(fontName, fontSize);
    
    TextureSamplerDescriptor samplerDesc = {
        ClampToEdge,
        ClampToEdge,
        Nearest,
        Nearest
    };
    _sampler = _graphicsDevice->makeTextureSampler(samplerDesc);
}

void StringRenderer::draw(const std::shared_ptr<CommandEncoder> &encoder,
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

void StringRenderer::rebuildVertexBuffer(String &string)
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
    glm::vec2 basePos = string.position;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        const Glyph &glyph = _glyphs[*c];
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

void StringRenderer::rebuildUniformBuffer(String &string,
                                          const glm::mat4x4 &projection)
{
    StringUniforms uniforms = {
        string.color,
        projection
    };
    string.uniforms->replace(sizeof(uniforms), &uniforms);
}

StringRenderer::StringHandle StringRenderer::add(const std::string &str,
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
    _strings.push_back(s);
    auto handle = _strings.end();
    --handle;
    
    const std::string &text = handle->contents;
    const size_t glyphCount = text.size();
    const size_t bytesPerVertex = sizeof(float) * 4;
    const size_t verticesPerGlyph = 6;
    const size_t vertexCount = verticesPerGlyph * glyphCount;
    const size_t bufferSize = bytesPerVertex * vertexCount;
    
    handle->vertexCount = vertexCount;
    handle->buffer = _graphicsDevice->makeBuffer(bufferSize, StaticDraw, ArrayBuffer);
    rebuildVertexBuffer(*handle);
    
    handle->uniforms = _graphicsDevice->makeBuffer(sizeof(StringUniforms), StaticDraw, UniformBuffer);
    rebuildUniformBuffer(*handle, glm::mat4x4());
    
    return handle;
}

void StringRenderer::remove(StringRenderer::StringHandle &handle)
{
    _strings.erase(handle);
}

void StringRenderer::replaceContents(StringHandle &handle,
                                     const std::string &contents)
{
    handle->contents = contents;
    rebuildVertexBuffer(*handle);
}
