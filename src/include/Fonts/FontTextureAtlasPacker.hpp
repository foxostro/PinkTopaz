//
//  FontTextureAtlasPacker.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef FontTextureAtlasPacker_hpp
#define FontTextureAtlasPacker_hpp

#include "Fonts/PackedGlyph.hpp"
#include "Fonts/Glyph.hpp"
#include <unordered_map>
#include <memory>
#include <array>
#include <boost/optional.hpp>

// Arranges glyphs to pack them into a texture atlas.
class FontTextureAtlasPacker
{
public:
    ~FontTextureAtlasPacker() = default;
    FontTextureAtlasPacker() = default;
    
    // Packs the glyphs into a texture atlas of the specified size.
    // Returns `none' if this is not possible.
    boost::optional<std::unordered_map<char, PackedGlyph>>
    packGlyphs(const std::vector<std::shared_ptr<Glyph>> &glyphs,
               size_t atlasSize);
    
private:
    struct Rect
    {
        glm::ivec2 origin, size;
        
        Rect() {}
        Rect(const glm::ivec2 &o, const glm::ivec2 &s) : origin(o), size(s) {}
    };
    
    class Node : public std::enable_shared_from_this<Node>
    {
    public:
        Node(const Rect &rect) : _rect(rect) {}
        
        // Insert a glyph into the tree and return the node that holds it.
        // Returns `none' if the glyph cannot fit into the tree.
        boost::optional<std::shared_ptr<Node>>
        insert(const std::shared_ptr<Glyph> &glyph);
        
        // Gets the glyph in this node, if there is one.
        inline const boost::optional<std::shared_ptr<Glyph>>& getGlyph() const
        {
            return _maybeGlyph;
        }
        
        inline const Rect& getRect() const
        {
            return _rect;
        }
        
    private:
        Rect _rect;
        boost::optional<std::array<std::shared_ptr<Node>, 2>> _maybeChildren;
        boost::optional<std::shared_ptr<Glyph>> _maybeGlyph;
    };
};

#endif /* FontTextureAtlasPacker_hpp */
