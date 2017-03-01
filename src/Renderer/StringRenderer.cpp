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
            
            TextureDescriptor desc;
            desc.type = Texture2D;
            desc.format = Red;
            desc.width = face->glyph->bitmap.width;
            desc.height = face->glyph->bitmap.rows;
            desc.unpackAlignment = 1;
            auto texture = _graphicsDevice->makeTexture(desc, face->glyph->bitmap.buffer);
            
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
    }
    
    void StringRenderer::draw(const glm::ivec4 &viewport)
    {
        auto encoder = _graphicsDevice->encoder(_renderPassDescriptor);
        encoder->setViewport(viewport);
        encoder->setShader(_shader);
        drawString(encoder,
                   "Pink Topaz / Ardent Storm",
                   glm::vec2(25.0f, 550.0f),
                   1.0f,
                   glm::vec3(1.0, 0.3f, 0.0f));
        _graphicsDevice->submit(encoder);
    }
    
    void StringRenderer::drawString(const std::shared_ptr<CommandEncoder> &encoder,
                                    const std::string &text,
                                    glm::vec2 basePos,
                                    float scale,
                                    const glm::vec3 &color)
    {
        _shader->setShaderUniform("textColor", color);
        
        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            const Glyph &glyph = _glyphs[*c];
            
            glm::vec2 offset(glyph.bearing.x * scale, (glyph.size.y - glyph.bearing.y) * -scale);
            glm::vec2 pos = basePos + offset;
            glm::vec2 size(glyph.size.x * scale, glyph.size.y * scale);
            
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
            basePos.x += (glyph.advance >> 6) * scale;
        }
    }
    
} // namespace PinkTopaz::Renderer
