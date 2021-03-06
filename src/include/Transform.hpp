//
//  Transform.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef Transform_hpp
#define Transform_hpp

#include <glm/mat4x4.hpp>

// Position and Orientation of an entity.
struct Transform
{
    Transform() {}
    Transform(const glm::mat4 &a) : value(a) {}
    glm::mat4 value;
};

#endif /* Transform_hpp */
