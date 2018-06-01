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

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

// Represents a 64-bit Morton Code (Z-order code) in three dimensions.
class Morton3
{
private:
    //                                        0123456789012345678901234567890123456789012345678901234567890123
    //                                        0         1         2         3         4         5         6
    static constexpr uint64_t MortonMaskX = 0b1001001001001001001001001001001001001001001001001001001001001001; // 22 bits
    static constexpr uint64_t MortonMaskY = 0b0010010010010010010010010010010010010010010010010010010010010010; // 21 bits
    static constexpr uint64_t MortonMaskZ = 0b0100100100100100100100100100100100100100100100100100100100100100; // 21 bits
    
    static constexpr uint64_t MortonOffX = 1;
    static constexpr uint64_t MortonOffY = 2;
    static constexpr uint64_t MortonOffZ = 4;
    
    uint64_t _mortonCode;
    
public:
    
#if defined(__BMI2__)
    
    static inline uint64_t encode(const glm::ivec3 &p)
    {
        // Check bounds to prevent overflow when using large coordinates.
        assert(p.x >= 0 && p.y >= 0 && p.z >= 0);
        assert(p.x < (1<<22) && p.y < (1<<21) && p.z < (1<<21));
        
        uint64_t morton = _pdep_u64((uint64_t)p.x, MortonMaskX)
                        | _pdep_u64((uint64_t)p.y, MortonMaskY)
                        | _pdep_u64((uint64_t)p.z, MortonMaskZ);
        
        return morton;
    }
    
    static inline void decode(glm::ivec3 &r, uint64_t morton)
    {
        r.x = (int)_pext_u64(morton, MortonMaskX);
        r.y = (int)_pext_u64(morton, MortonMaskY);
        r.z = (int)_pext_u64(morton, MortonMaskZ);
    }
    
    static inline glm::ivec3 decode(uint64_t morton)
    {
        glm::ivec3 r;
        r.x = (int)_pext_u64(morton, MortonMaskX);
        r.y = (int)_pext_u64(morton, MortonMaskY);
        r.z = (int)_pext_u64(morton, MortonMaskZ);
        return r;
    }
    
#else
    
    static inline uint64_t encode(const glm::ivec3 &p)
    {
        // See <http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/> for details.
        uint64_t x = p.x, y = p.y, z = p.z;
        uint64_t answer = 0;
        for (uint64_t i = 0; i < (sizeof(uint64_t)* CHAR_BIT)/3; ++i) {
            answer |= ((x & ((uint64_t)1 << i)) << 2*i) | ((y & ((uint64_t)1 << i)) << (2*i + 1)) | ((z & ((uint64_t)1 << i)) << (2*i + 2));
        }
        return answer;
    }
    
    static inline void decode(glm::ivec3 &r, uint64_t morton)
    {
        // See <http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/> for details.
        
        uint64_t x = 0, y = 0, z = 0;
        const unsigned checkbits = (unsigned)(floor((sizeof(uint64_t) * 8.0f / 3.0f)));
        
        for (unsigned i = 0; i <= checkbits; ++i) {
            uint64_t selector = 1;
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

    static inline glm::ivec3 decode(uint64_t morton)
    {
        glm::ivec3 result;
        decode(result, morton);
        return result;
    }

#endif /* defined(__BMI2__) */
    
    ~Morton3() = default;
    
    // Default constructor
    Morton3() : _mortonCode(0) {}

    // Constructor.
    template<typename T>
    Morton3(T mortonCode) : _mortonCode(mortonCode) {}
    
    // Construct by encoding the specified integer coordinates as a Morton Code.
    Morton3(const glm::ivec3 &p) : _mortonCode(Morton3::encode(p)) {}
    
    // Copy-assignment operator
    Morton3& operator=(const Morton3 &rhs)
    {
        _mortonCode = rhs._mortonCode;
        return *this;
    }
    
    // Cast to uint64_t
    explicit operator uint64_t() const
    {
        return _mortonCode;
    }
    
    // Cast to size_t
    explicit operator size_t() const
    {
        return _mortonCode;
    }
    
    bool operator==(const Morton3 &right) const
    {
        return _mortonCode == right._mortonCode;
    }
    
    inline bool operator!=(const Morton3 &right) const
    {
        return _mortonCode != right._mortonCode;
    }
    
    inline bool operator<(const Morton3 &right) const
    {
        return _mortonCode < right._mortonCode;
    }
    
    inline bool operator<=(const Morton3 &right) const
    {
        return _mortonCode <= right._mortonCode;
    }
    
    inline bool operator>(const Morton3 &right) const
    {
        return _mortonCode > right._mortonCode;
    }
    
    inline bool operator>=(const Morton3 &right) const
    {
        return _mortonCode > right._mortonCode;
    }
    
    // Decode the Morton Code to get back the integer coordinates for it.
    inline void decode(glm::ivec3 &r) const
    {
        return decode(r, _mortonCode);
    }
    
    // Decode the Morton Code to get back the integer coordinates for it.
    inline glm::ivec3 decode() const
    {
        glm::ivec3 r;
        decode(r);
        return r;
    }
    
    // Increment the X component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 incX()
    {
        const uint64_t x_sum = ((_mortonCode | (MortonMaskY | MortonMaskZ)) + MortonOffX);
        const uint64_t r = (x_sum & MortonMaskX) | (_mortonCode & (MortonMaskY | MortonMaskZ));
        _mortonCode = r;
        return *this;
    }
    
    // Increment the Y component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 incY()
    {
        const uint64_t y_sum = ((_mortonCode | (MortonMaskX | MortonMaskZ)) + MortonOffY);
        const uint64_t r = (y_sum & MortonMaskY) | (_mortonCode & (MortonMaskX | MortonMaskZ));
        _mortonCode = r;
        return *this;
    }
    
    // Increment the Z component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 incZ()
    {
        const uint64_t z_sum = ((_mortonCode | (MortonMaskX | MortonMaskY)) + MortonOffZ);
        const uint64_t r = ((z_sum & MortonMaskZ) | (_mortonCode & (MortonMaskX | MortonMaskY)));
        _mortonCode = r;
        return *this;
    }
    
    // Decrement the X component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 decX()
    {
        const uint64_t x_diff = (_mortonCode & MortonMaskX) - MortonOffX;
        const uint64_t r = ((x_diff & MortonMaskX) | (_mortonCode & (MortonMaskY | MortonMaskZ)));
        _mortonCode = r;
        return *this;
    }
    
    // Decrement the Y component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 decY()
    {
        const uint64_t y_diff = (_mortonCode & MortonMaskY) - MortonOffY;
        const uint64_t r = ((y_diff & MortonMaskY) | (_mortonCode & (MortonMaskX | MortonMaskZ)));
        _mortonCode = r;
        return *this;
    }
    
    // Decrement the Z component of this Morton Code by one. Avoids a full
    // decode and re-encode by taking advantage of clever math.
    // See <http://bitmath.blogspot.fr/2012/11/tesseral-arithmetic-useful-snippets.html>
    inline Morton3 decZ()
    {
        const uint64_t z_diff = (_mortonCode & MortonMaskZ) - MortonOffZ;
        const uint64_t r = ((z_diff & MortonMaskZ) | (_mortonCode & (MortonMaskX | MortonMaskY)));
        _mortonCode = r;
        return *this;
    }
};

namespace std {
    template <> struct hash<Morton3>
    {
        size_t operator()(const Morton3 &morton) const
        {
            return hash<size_t>()((size_t)morton);
        }
    };
}

#endif /* Morton_hpp */
