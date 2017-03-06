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
#include <glm/gtx/matrix_decompose.hpp>

namespace PinkTopaz {

    // Position and Orientation of an entity.
    struct Transform
    {
        Transform() {}
        
        Transform(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
        {
            glm::mat4 transformation = glm::lookAt(eye, center, up);
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transformation, scale, rotation, position, skew, perspective);
            
            rotation = glm::conjugate(rotation);
        }
        
        glm::mat4x4 getMatrix() const
        {
            glm::mat4 m;
            m *= glm::mat4_cast(rotation);
            m = glm::translate(m, position);
            return m;
        }
        
        glm::vec3 position;
        glm::quat rotation;
    };

} // namespace PinkTopaz

#endif /* Transform_hpp */
