//
//  Morton.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/16/17.
//
//

#ifndef Morton_hpp
#define Morton_hpp

#include <cassert>
#include <cstdint>
#include <glm/glm.hpp>

#if defined(__BMI2__)
#include <x86intrin.h>
#endif

// Represents a 32-bit Morton Code (Z-order code) in three dimensions.
class Morton3
{
private:
    static constexpr uint32_t MortonMaskX = 0b01001001001001001001001001001001;
    static constexpr uint32_t MortonMaskY = 0b10010010010010010010010010010010;
    static constexpr uint32_t MortonMaskZ = 0b00100100100100100100100100100100;
    
    uint32_t _mortonCode;
    
public:
    
#if defined(__BMI2__)
    
    static inline uint32_t encode(const glm::ivec3 &p)
    {
        // Check bounds to prevent overflow when using large coordinates.
        assert(p.x >= 0 && p.y >= 0 && p.z >= 0);
        assert(p.x < (1<<21) && p.y < (1<<21) && p.z < (1<<21));
        
        uint32_t morton = _pdep_u32((uint32_t)p.x, MortonMaskX)
                        | _pdep_u32((uint32_t)p.y, MortonMaskY)
                        | _pdep_u32((uint32_t)p.z, MortonMaskZ);
        
        return morton;
    }
    
    static inline void decode(glm::ivec3 &r, uint32_t morton)
    {
        r.x = _pext_u32(morton, MortonMaskX);
        r.y = _pext_u32(morton, MortonMaskY);
        r.z = _pext_u32(morton, MortonMaskZ);
    }
    
    static inline glm::ivec3 decode(uint32_t morton)
    {
        glm::ivec3 r;
        r.x = _pext_u32(morton, MortonMaskX);
        r.y = _pext_u32(morton, MortonMaskY);
        r.z = _pext_u32(morton, MortonMaskZ);
        return r;
    }
    
#else
    
    static inline uint32_t encode(const glm::ivec3 &p)
    {
        // See <http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/> for details.
        uint32_t x = p.x, y = p.y, z = p.z;
        uint32_t answer = 0;
        for (uint32_t i = 0; i < (sizeof(uint32_t)* CHAR_BIT)/3; ++i) {
            answer |= ((x & ((uint32_t)1 << i)) << 2*i) | ((y & ((uint32_t)1 << i)) << (2*i + 1)) | ((z & ((uint32_t)1 << i)) << (2*i + 2));
        }
        return answer;
    }
    
    static inline void decode(glm::ivec3 &r, uint32_t morton)
    {
        // See <http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/> for details.
        
        uint32_t x = 0, y = 0, z = 0;
        const unsigned checkbits = (unsigned)(floor((sizeof(uint32_t) * 8.0f / 3.0f)));
        
        for (unsigned i = 0; i <= checkbits; ++i) {
            uint32_t selector = 1;
            unsigned shift_selector = 3 * i;
            unsigned shiftback = 2 * i;
            x |= (morton & (selector << shift_selector)) >> (shiftback);
            y |= (morton & (selector << (shift_selector + 1))) >> (shiftback + 1);
            z |= (morton & (selector << (shift_selector + 2))) >> (shiftback + 2);
        }
        
        r.x = x;
        r.y = y;
        r.z = z;
    }
    
#endif /* defined(__BMI2__) */
    
    ~Morton3() = default;
    Morton3() : _mortonCode(0) {}
    Morton3(uint32_t mortonCode) : _mortonCode(mortonCode) {}
    Morton3(size_t mortonCode) : _mortonCode(mortonCode) {}
    Morton3(const glm::ivec3 &p) : _mortonCode(Morton3::encode(p)) {}
    
    explicit operator uint32_t() const
    {
        return _mortonCode;
    }
    
    explicit operator size_t() const
    {
        return _mortonCode;
    }
    
    inline void decode(glm::ivec3 &r) const
    {
        return decode(r, _mortonCode);
    }
    
    inline glm::ivec3 decode() const
    {
        glm::ivec3 r;
        decode(r);
        return r;
    }
    
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline void incX()
    {
        const uint32_t x_sum = ((_mortonCode | (MortonMaskY | MortonMaskZ)) + 4);
        const uint32_t r = (x_sum & MortonMaskX) | (_mortonCode & (MortonMaskY | MortonMaskZ));
        _mortonCode = r;
    }
    
    inline void incY()
    {
        const uint32_t y_sum = ((_mortonCode | (MortonMaskX | MortonMaskZ)) + 2);
        const uint32_t r = (y_sum & MortonMaskY) | (_mortonCode & (MortonMaskX | MortonMaskZ));
        _mortonCode = r;
    }
    
    inline void incZ()
    {
        const uint32_t z_sum = ((_mortonCode | (MortonMaskX | MortonMaskY)) + 1);
        const uint32_t r = ((z_sum & MortonMaskZ) | (_mortonCode & (MortonMaskX | MortonMaskY)));
        _mortonCode = r;
    }
    
    inline void decX()
    {
        const uint32_t x_diff = (_mortonCode & MortonMaskX) - 4;
        const uint32_t r = ((x_diff & MortonMaskX) | (_mortonCode & (MortonMaskY | MortonMaskZ)));
        _mortonCode = r;
    }
    
    inline void decY()
    {
        const uint32_t y_diff = (_mortonCode & MortonMaskY) - 2;
        const uint32_t r = ((y_diff & MortonMaskY) | (_mortonCode & (MortonMaskX | MortonMaskZ)));
        _mortonCode = r;
    }
    
    inline void decZ()
    {
        const uint32_t z_diff = (_mortonCode & MortonMaskZ) - 1;
        const uint32_t r = ((z_diff & MortonMaskZ) | (_mortonCode & (MortonMaskX | MortonMaskY)));
        _mortonCode = r;
    }
};

#endif /* Morton_hpp */
