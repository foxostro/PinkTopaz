//
//  FontTextureAtlasPacker.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Fonts/FontTextureAtlasPacker.hpp"
#include <vector>
#include <list>

boost::optional<std::shared_ptr<FontTextureAtlasPacker::Node>>
FontTextureAtlasPacker::Node::insert(const std::shared_ptr<Glyph> &glyph)
{
    if (_maybeChildren) {
        for (std::shared_ptr<Node> &child : *_maybeChildren) {
            assert(child);
            auto result = child->insert(glyph);
            if (result) {
                return result;
            }
        }
        
        return boost::none; // Doesn't fit.
    } else {
        // If this node is already occupied then return.
        if (_maybeGlyph) {
            return boost::none;
        }
        
        // If this node is too small for the glyph then return.
        const glm::ivec2 glyphSize = glyph->getSize();
        if (glyphSize.x > _rect.size.x || glyphSize.y > _rect.size.y) {
            return boost::none;
        }

        // If the node is perfectly sized to accomodate the glyph then great.
        if (glyphSize.x == _rect.size.x && glyphSize.y == _rect.size.y) {
            _maybeGlyph = boost::make_optional(glyph);
            return shared_from_this();
        }
        
        // Otherwise, split the node.
        // First, decide how to split.
        int dw = _rect.size.x - glyphSize.x;
        int dh = _rect.size.y - glyphSize.y;
        
        Rect rect0, rect1;
        
        if (dw > dh) {
            // Split along a line drawn vertically through the node.
            rect0 = Rect(_rect.origin,
                         glm::ivec2(glyphSize.x,
                                    _rect.size.y));
            rect1 = Rect(glm::ivec2(_rect.origin.x + glyphSize.x,
                                    _rect.origin.y),
                         glm::ivec2(_rect.size.x - glyphSize.x,
                                    _rect.size.y));
            assert(rect0.origin.x + rect0.size.x == rect1.origin.x);
            assert(rect0.size.x + rect1.size.x == _rect.size.x);
        } else {
            // Split along a line drawn horizontally through the node.
            rect0 = Rect(_rect.origin,
                         glm::ivec2(_rect.size.x,
                                    glyphSize.y));
            rect1 = Rect(glm::ivec2(_rect.origin.x,
                                    _rect.origin.y + glyphSize.y),
                         glm::ivec2(_rect.size.x,
                                    _rect.size.y - glyphSize.y));
            assert(rect0.origin.y + rect0.size.y == rect1.origin.y);
            assert(rect0.size.y + rect1.size.y == _rect.size.y);
        }
        
        // Actually create the child nodes now.
        _maybeChildren = boost::make_optional(std::array<std::shared_ptr<Node>, 2>());
        _maybeChildren->at(0) = std::make_shared<Node>(rect0);
        _maybeChildren->at(1) = std::make_shared<Node>(rect1);
        
        // Finally, insert on the first child.
        return _maybeChildren->at(0)->insert(glyph);
    }
}

boost::optional<std::unordered_map<char, PackedGlyph>>
FontTextureAtlasPacker::packGlyphs(const std::vector<std::shared_ptr<Glyph>> &glyphs,
                                   size_t atlasSize)
{
    std::unordered_map<char, PackedGlyph> packedGlyphs;
    std::shared_ptr<Node> root = std::make_shared<Node>(Rect(glm::ivec2(0, 0), glm::ivec2(atlasSize, atlasSize)));
    
    for (const auto &glyph : glyphs) {
        auto maybeNode = root->insert(glyph);
        if (!maybeNode) {
            return boost::none; // Doesn't fit.
        }
        
        const Rect &rect = (*maybeNode)->getRect();
        packedGlyphs[glyph->getCharCode()] = {
            glm::vec2((float)rect.origin.x / atlasSize,
                      (float)rect.origin.y / atlasSize),
            glm::vec2((float)rect.size.x / atlasSize,
                      (float)rect.size.y / atlasSize),
            glyph->getSize(),
            glyph->getBearing(),
            glyph->getAdvance()
        };
    }
    
    return packedGlyphs;
}
