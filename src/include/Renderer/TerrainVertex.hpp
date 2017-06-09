//
//  TerrainVertex.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#ifndef TerrainVertex_hpp
#define TerrainVertex_hpp

#include <glm/glm.hpp>

struct TerrainVertex
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec3 texCoord;
    
    TerrainVertex() : position(0, 0, 0, 0), color(0, 0, 0, 0), texCoord(0, 0, 0) {}
    
    TerrainVertex(const glm::vec4 &p, const glm::vec4 &c, const glm::vec3 &t)
     : position(p), color(c), texCoord(t)
    {}
    
    inline bool operator==(const TerrainVertex &other) const
    {
        const bool equal = position == other.position
                        && colorsAreEqual(color, other.color)
                        && texCoord == other.texCoord;
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

struct TerrainUniforms
{
    glm::mat4 view, proj;
};

#endif /* TerrainVertex_hpp */
