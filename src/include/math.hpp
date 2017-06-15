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

#if __has_include(<x86intrin.h>)
#include <x86intrin.h> // for __builtin_clzll
static inline unsigned ilog2(unsigned x)
{
    // See <https://stackoverflow.com/a/11376759/2403342> for details.
    return (unsigned)(8*sizeof(unsigned long long) - __builtin_clzll((x)) - 1);
}
#else
#error AFOX_TODO: implement ilog2() for MSVC
#endif

static inline bool isPowerOfTwo(unsigned x)
{
    return (x & (x - 1)) == 0;
}

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
