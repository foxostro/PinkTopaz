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

#if defined(_MSC_VER)
#include <intrin.h>
#define __lzcnt32 _lzcnt_u32
#else
#include <x86intrin.h>
#endif

#if defined(__LZCNT__)
inline uint32_t ilog2(uint32_t x)
{
    // See <https://stackoverflow.com/a/11376759/2403342> for details.
    return (uint32_t)(8*sizeof(unsigned long long) - __lzcnt64(x) - 1);
}
#else
inline uint32_t ilog2(uint32_t x)
{
    // See <http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious>
    uint32_t r = 0;
    while (x >>= 1) {
        r++;
    }
    return r;
}
#endif // defined(__LZCNT__)

#if defined(__LZCNT__)
inline uint32_t roundUpToPowerOfTwo(uint32_t x)
{
    return 1 << (32 - __lzcnt32(x - 1));
}
#else
inline uint32_t roundUpToPowerOfTwo(uint32_t v)
{
    // See <http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2>
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
#endif // defined(__LZCNT__)

#if defined(__POPCNT__)
inline bool isPowerOfTwo(uint32_t x)
{
    return _mm_popcnt_u32(x) == 1;
}
#else
inline bool isPowerOfTwo(uint32_t x)
{
    // See <http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2>
    return (x && !(x & (x - 1)));
}
#endif // defined(__POPCNT__)

template<typename T> inline
const T& clamp(const T &value, const T &min, const T &max)
{
    return std::min(std::max(value, min), max);
}

inline float dot3(const glm::vec3 &a, const glm::vec4 &b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

#endif /* math_hpp */
