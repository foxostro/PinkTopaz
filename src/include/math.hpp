//
//  math.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#ifndef math_hpp
#define math_hpp

#include <algorithm>
#include <glm/glm.hpp>

template<typename T>
static inline const T& clamp(const T &value, const T &min, const T &max)
{
    return std::min(std::max(value, min), max);
}

static inline float dot3(const glm::vec3 &a, const glm::vec4 &b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

#endif /* math_hpp */
