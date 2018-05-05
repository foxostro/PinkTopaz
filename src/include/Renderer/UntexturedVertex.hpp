//
//  UntexturedVertex.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/4/18.
//
//

#ifndef UntexturedVertex_hpp
#define UntexturedVertex_hpp

#include <glm/glm.hpp>

struct UntexturedVertex
{
    glm::vec4 position;
    glm::vec4 color;
    
    UntexturedVertex() : position(0, 0, 0, 0), color(0, 0, 0, 0) {}
    
    UntexturedVertex(const glm::vec4 &p, const glm::vec4 &c)
     : position(p), color(c)
    {}
    
    inline bool operator==(const UntexturedVertex &other) const
    {
        const bool equal = (position == other.position) && colorsAreEqual(color, other.color);
        return equal;
    }
    
private:
    inline bool colorsAreEqual(const glm::vec4 &a, const glm::vec4 &b) const
    {
        // When we serialize mesh colors, they are converted to integers in the
        // range of [0,255]. So, we need to check to see if the two are "close
        // enough" to be equivalent.
        glm::ivec4 c(a.r * 255, a.g * 255, a.b * 255, a.a * 255);
        glm::ivec4 d(b.r * 255, b.g * 255, b.b * 255, b.a * 255);
        const bool equal = (c == d);
        return equal;
    }
};

struct alignas(16) UntexturedUniforms
{
    glm::mat4 view, proj;
};

#endif /* UntexturedVertex_hpp */
