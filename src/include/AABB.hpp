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
#include <vector>
#include <sstream>
#include <array>

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
    
    std::string to_string() const
    {
        std::ostringstream ss;
        ss << "{("    << center.x << ", " << center.y << ", " << center.z
           << ") x (" << extent.x*2 << ", " << extent.y*2 << ", " << extent.z*2
           << ")}";
        return ss.str();
    }
};

typedef _AABB<glm::vec3> AABB;

template<typename PointType>
static inline bool
isPointInsideBox(const PointType &point,
                 const PointType &mins,
                 const PointType &maxs)
{
    return point.x >= mins.x && point.y >= mins.y && point.z >= mins.z &&
           point.x < maxs.x && point.y < maxs.y && point.z < maxs.z;
}


static inline bool
isPointInsideBox(const glm::vec3 &point, const AABB &box)
{
    return isPointInsideBox(point, box.mins(), box.maxs());
}


static inline bool
doBoxesIntersect(const AABB &a, const AABB &b)
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
