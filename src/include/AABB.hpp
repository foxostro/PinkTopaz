//
//  AABB.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef AABB_hpp
#define AABB_hpp

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <sstream>
#include <array>
#include <boost/functional/hash.hpp>
#include "CerealGLM.hpp"
#include <spdlog/fmt/ostr.h>

// An axis-aligned bounding box.
template<typename TYPE>
struct _AABB
{
    // The point at the exact center of the bounding box.
    TYPE center;
        
    // The corners of the box are given given as the Cartesian Product of
    // {-a.x, -a.y, -a.z} and {+a.x, +a.y, +a.z} where a = center + extent.
    // This means that the length of the edge of the box along the X axis is
    // extent.x * 2, and ditto for the other two axii.
    TYPE extent;
    
    bool operator==(const _AABB<TYPE> &other) const
    {
        return (center == other.center) && (extent == other.extent);
    }
    
    bool operator!=(const _AABB<TYPE> &other) const
    {
        bool theSame = ((*this) == other);
        return !theSame;
    }
    
    bool operator<(const _AABB<TYPE> &right) const
    {
        return center.x < right.center.x; // This is arbitrary, but consistent.
    }
    
    inline TYPE mins() const
    {
        return center - extent;
    }
    
    inline TYPE maxs() const
    {
        return center + extent;
    }
    
    inline _AABB<TYPE> inset(TYPE inset) const
    {
        return _AABB<TYPE>{center, extent - inset};
    }
    
    auto octants() const
    {
        const glm::vec3 subExtent = extent * 0.50f;
        const std::array<_AABB<TYPE>, 8> r = {{
            { center + glm::vec3(-subExtent.x, -subExtent.y, -subExtent.z), subExtent},
            { center + glm::vec3(-subExtent.x, -subExtent.y, +subExtent.z), subExtent},
            { center + glm::vec3(-subExtent.x, +subExtent.y, -subExtent.z), subExtent},
            { center + glm::vec3(-subExtent.x, +subExtent.y, +subExtent.z), subExtent},
            { center + glm::vec3(+subExtent.x, -subExtent.y, -subExtent.z), subExtent},
            { center + glm::vec3(+subExtent.x, -subExtent.y, +subExtent.z), subExtent},
            { center + glm::vec3(+subExtent.x, +subExtent.y, -subExtent.z), subExtent},
            { center + glm::vec3(+subExtent.x, +subExtent.y, +subExtent.z), subExtent}
        }};
        return r;
    }
    
    inline _AABB<TYPE> intersect(const _AABB<TYPE> &thatBox) const
    {
        const TYPE thisMin = mins();
        const TYPE thatMin = thatBox.mins();
        const TYPE thisMax = maxs();
        const TYPE thatMax = thatBox.maxs();
        const TYPE min = TYPE(std::max(thisMin.x, thatMin.x),
                              std::max(thisMin.y, thatMin.y),
                              std::max(thisMin.z, thatMin.z));
        const TYPE max = TYPE(std::min(thisMax.x, thatMax.x),
                              std::min(thisMax.y, thatMax.y),
                              std::min(thisMax.z, thatMax.z));
        const TYPE center = (max + min) * 0.5f;
        const TYPE extent = (max - min) * 0.5f;
        return {center, extent};
    }
    
    inline _AABB<TYPE> unionBox(const _AABB<TYPE> &thatBox) const
    {
        const TYPE thisMin = mins();
        const TYPE thatMin = thatBox.mins();
        const TYPE thisMax = maxs();
        const TYPE thatMax = thatBox.maxs();
        const TYPE min = TYPE(std::min(thisMin.x, thatMin.x),
                              std::min(thisMin.y, thatMin.y),
                              std::min(thisMin.z, thatMin.z));
        const TYPE max = TYPE(std::max(thisMax.x, thatMax.x),
                              std::max(thisMax.y, thatMax.y),
                              std::max(thisMax.z, thatMax.z));
        const TYPE center = (max + min) * 0.5f;
        const TYPE extent = (max - min) * 0.5f;
        return {center, extent};
    }
    
    // Return the string representation of this bounding box.
    std::string to_string() const
    {
        std::ostringstream ss;
        ss << "{"
           << glm::to_string(mins())
           << " x "
           << glm::to_string(maxs())
           << "}";
        return ss.str();
    }
    
    // Permits logging with spdlog.
    template<typename OStream>
    friend OStream& operator<<(OStream &os, const _AABB<TYPE> &box)
    {
        os << "{"
           << glm::to_string(box.mins())
           << " x "
           << glm::to_string(box.maxs())
           << "}";
        return os;
    }
    
    // Permits serialization with cereal.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(center), CEREAL_NVP(extent));
    }
};

using AABB = _AABB<glm::vec3>;

namespace std {
    template <> struct hash<glm::vec3>
    {
        size_t operator()(const glm::vec3 &point) const
        {
            size_t seed = 0;
            boost::hash_combine(seed, point.x);
            boost::hash_combine(seed, point.y);
            boost::hash_combine(seed, point.z);
            return seed;
        }
    };
}

namespace std {
    template <> struct hash<AABB>
    {
        size_t operator()(const AABB &box) const
        {
            size_t seed = 0;
            std::hash<glm::vec3> vecHasher;
            boost::hash_combine(seed, vecHasher(box.center));
            boost::hash_combine(seed, vecHasher(box.extent));
            return seed;
        }
    };
}

template<typename PointType> inline bool
isPointInsideBox(const PointType &point,
                 const PointType &mins,
                 const PointType &maxs)
{
    return point.x >= mins.x && point.y >= mins.y && point.z >= mins.z &&
           point.x < maxs.x && point.y < maxs.y && point.z < maxs.z;
}


inline bool isPointInsideBox(const glm::vec3 &point, const AABB &box)
{
    return isPointInsideBox(point, box.mins(), box.maxs());
}


inline bool doBoxesIntersect(const AABB &a, const AABB &b)
{
    const glm::vec3 a_max = a.maxs();
    const glm::vec3 b_max = b.maxs();
    
    const glm::vec3 a_min = a.mins();
    const glm::vec3 b_min = b.mins();
    
    return (a_max.x >= b_min.x) &&
           (a_min.x <= b_max.x) &&
           (a_max.y >= b_min.y) &&
           (a_min.y <= b_max.y) &&
           (a_max.z >= b_min.z) &&
           (a_min.z <= b_max.z);
}

#endif /* AABB_hpp */
