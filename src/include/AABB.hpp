//
//  AABB.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef AABB_hpp
#define AABB_hpp

#include <glm/vec3.hpp>

namespace PinkTopaz {
    
    // An axis-aligned bounding box.
    template<typename TYPE>
    struct _AABB
    {
        // The point at the exact center of the bounding box.
        TYPE center;
        
        // The corners of the box are given given as the Cartesian Product of
        // {-a.x, -a.y, -a.z} and {+a.x, +a.y, +a.z} where a = center + extent.
        // This means that the length of the edge of the box along the X axis is
        // extent.x * 2, and ditto for the other two axii.
        TYPE extent;
    };
    
    typedef _AABB<glm::vec3> AABB;

} // namespace PinkTopaz

#endif /* AABB_hpp */
