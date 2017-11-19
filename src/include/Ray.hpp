//
//  Ray.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/18/17.
//
//

#ifndef Ray_hpp
#define Ray_hpp

#include <glm/vec3.hpp>

class Ray
{
public:
    glm::vec3 origin, direction;
    
    Ray() = default;
    
    Ray(const glm::vec3 &myOrigin, const glm::vec3 &myDirection)
     : origin(myOrigin), direction(myDirection)
    {}
    
    bool operator==(const Ray &a) const
    {
        return origin == a.origin && direction == a.direction;
    }
    
    bool operator!=(const Ray &a) const
    {
        return !(*this == a);
    }
};

#endif /* Ray_hpp */
