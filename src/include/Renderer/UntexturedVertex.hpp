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
    
    UntexturedVertex() : position(0, 0, 0, 0) {}
    
    UntexturedVertex(const glm::vec4 &p)
     : position(p)
    {}
    
    inline bool operator==(const UntexturedVertex &other) const
    {
        const bool equal = (position == other.position);
        return equal;
    }
};

struct alignas(16) UntexturedUniforms
{
    glm::mat4 view, proj;
    glm::vec4 color;
};

#endif /* UntexturedVertex_hpp */
