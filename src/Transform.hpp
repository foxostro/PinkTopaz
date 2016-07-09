//
//  Transform.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef Transform_hpp
#define Transform_hpp

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // for lookAt

namespace PinkTopaz {

    // Position and Orientation of an entity.
    struct Transform
    {
        Transform() {}
        
        Transform(const glm::mat4x4 &val) : value(val) {}
        
        Transform(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
        {
            value = glm::lookAt(eye, center, up);
        }
        
        glm::mat4x4 value;
    };

} // namespace PinkTopaz

#endif /* Transform_hpp */
