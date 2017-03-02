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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SDL.h"

namespace PinkTopaz::Renderer {
    
    StringRenderer::StringRenderer(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                                   const std::string &fontName,
                                   unsigned fontSize)
     : _graphicsDevice(graphicsDevice),
       _canvasSize(800, 600)
    {
        _renderPassDescriptor.blend = true;
        _renderPassDescriptor.depthTest = false;
        _renderPassDescriptor.clear = false;
        
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            throw Exception("Failed to initialize Freetype.");
        }
        
        FT_Face face;
        if (FT_New_Face(ft, fontName.c_str(), 0, &face)) {
            throw Exception("Failed to load the Vegur font.");
        }
        
        if (FT_Set_Pixel_Sizes(face, 0, fontSize)) {
            throw Exception("Failed to set the font size.");
        }
        
        for (FT_ULong c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                throw Exception("Failed to load the glyph %c.", (char)c);
            }
            
            TextureDescriptor texDesc = {
                .type = Texture2D,
                .format = R8,
                .width = face->glyph->bitmap.width,
                .height = face->glyph->bitmap.rows,
                .depth = 1,
                .unpackAlignment = 1,
                .generateMipMaps = false
            };
            auto texture = _graphicsDevice->makeTexture(texDesc, face->glyph->bitmap.buffer);
            
            // Now store character for later use
            Glyph glyph = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                (unsigned)face->glyph->advance.x
            };
            _glyphs.insert(std::pair<char, Glyph>((char)c, glyph));
        }
        
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        
        _shader = _graphicsDevice->makeShader("text_vert", "text_frag");
        glm::mat4 projection = glm::ortho(0.0f, (float)_canvasSize.x,
                                          0.0f, (float)_canvasSize.y);
        _shader->setShaderUniform("projection", projection);
        _shader->setShaderUniform("tex", 0);
        
        const size_t vertexSize = sizeof(float) * 4;
        const size_t vertexCount = 6;
        
        VertexFormat format;
        format.attributes.emplace_back((AttributeFormat){
            .size = 4,
            .type = AttributeTypeFloat,
            .normalized = false,
            .stride = vertexSize,
            .offset = 0
        });
        _buffer = _graphicsDevice->makeBuffer(format,
                                              vertexSize * vertexCount,
                                              vertexCount,
                                              DynamicDraw);
        
        TextureSamplerDescriptor samplerDesc = {
            .addressS = Renderer::ClampToEdge,
            .addressT = Renderer::ClampToEdge,
            .minFilter = Renderer::Nearest,
            .maxFilter = Renderer::Nearest
        };
        _sampler = _graphicsDevice->makeTextureSampler(samplerDesc);
    }
    
    void StringRenderer::draw(const glm::ivec4 &viewport)
    {
        if (_viewport != viewport) {
            _viewport = viewport;
            
            glm::mat4 projection = glm::ortho((float)viewport.x,
                                              (float)viewport.x + viewport.z,
                                              (float)viewport.y,
                                              (float)viewport.y + viewport.w);
            _shader->setShaderUniform("projection", projection);
        }
        
        auto encoder = _graphicsDevice->encoder(_renderPassDescriptor);
        encoder->setViewport(viewport);
        encoder->setShader(_shader);
        encoder->setFragmentSampler(_sampler, 0);
        
        for (auto &string : _strings)
        {
            drawString(encoder,
                       string.getContents(),
                       string.getPosition(),
                       glm::vec3(0.0f, 0.0f, 0.0f));
        }

        _graphicsDevice->submit(encoder);
    }
    
    void StringRenderer::drawString(const std::shared_ptr<CommandEncoder> &encoder,
                                    const std::string &text,
                                    glm::vec2 basePos,
                                    const glm::vec3 &color)
    {
        _shader->setShaderUniform("textColor", color);
        
        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            const Glyph &glyph = _glyphs[*c];
            
            glm::vec2 offset(glyph.bearing.x, glyph.bearing.y - glyph.size.y);
            glm::vec2 pos = basePos + offset;
            glm::vec2 size(glyph.size.x, glyph.size.y);
            
            // Update the vertex buffer for each glyph.
            const size_t numVertices = 6;
            float vertexBytes[numVertices][4] = {
                { pos.x,          pos.y + size.y,   0.0, 0.0 },
                { pos.x,          pos.y,            0.0, 1.0 },
                { pos.x + size.x, pos.y,            1.0, 1.0 },
                
                { pos.x,          pos.y + size.y,   0.0, 0.0 },
                { pos.x + size.x, pos.y,            1.0, 1.0 },
                { pos.x + size.x, pos.y + size.y,   1.0, 0.0 }
            };
            encoder->setVertexBytes(_buffer, sizeof(vertexBytes), vertexBytes);
            encoder->setFragmentTexture(glyph.texture, 0);
            encoder->drawPrimitives(Triangles, 0, numVertices, 1);
            
            // Advance to the next glyph.
            basePos.x += glyph.advance / 64.0f;
        }
    }
    
    StringRenderer::StringHandle StringRenderer::add(const String &string)
    {
        _strings.push_back(string);
        auto iter = _strings.end();
        --iter;
        return iter;
    }
    
    void StringRenderer::remove(StringRenderer::StringHandle &handle)
    {
        _strings.erase(handle);
    }
    
    void StringRenderer::replaceContents(StringHandle &handle, const std::string &contents)
    {
        String &string = *handle;
        string.setContents(contents);
    }
    
} // namespace PinkTopaz::Renderer
