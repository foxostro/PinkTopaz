//
//  Plane.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/14/17.
//
//

#ifndef Plane_hpp
#define Plane_hpp

#include <glm/glm.hpp>

// Represents a plane through the coefficients of the plane equation.
typedef glm::vec4 Plane;

inline float signedDistance(const Plane &plane, const glm::vec3 &point)
{
    const glm::vec3 normal(plane.x, plane.y, plane.z);
    const float d = glm::dot(normal, point) + plane.w;
    return d;
}

#endif /* Plane_hpp */
