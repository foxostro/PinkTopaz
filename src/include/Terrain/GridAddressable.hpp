//
//  GridAddressable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/20/17.
//
//

#ifndef GridAddressable_hpp
#define GridAddressable_hpp

#include "AABB.hpp"
#include "Exception.hpp"
#include "Frustum.hpp"
#include <glm/vec3.hpp>
#include <functional>

// Exception thrown when attempting to access the grid at a point that is not in
// the valid space of the grid.
class OutOfBoundsException : public Exception
{
public:
    OutOfBoundsException() : Exception("out of bounds") {}
};

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class GridAddressable
{
public:
#ifndef NDEBUG
    static constexpr bool EnableVerboseBoundsChecking = true;
#else
    static constexpr bool EnableVerboseBoundsChecking = false;
#endif
    
    virtual ~GridAddressable() = default;
    
    // Get the object corresponding to the specified point in space.
    // Note that each point in space corresponds to exactly one cell.
    // Throws an exception if the point is not within this grid.
    virtual const TYPE& get(const glm::vec3 &p) const = 0;
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    virtual glm::vec3 cellDimensions() const = 0;
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    virtual AABB boundingBox() const = 0;
    
    // Gets the number of cells along each axis within the valid region.
    virtual glm::ivec3 gridResolution() const = 0;
    
    // Gets the coordinates of the cell in which the specified point resides.
    // These integer coordinates can be used to locate the cell within the grid.
    //
    // This method will not throw an exception if the point is outside the valid
    // space of the grid. In this case, you will receive garbage results, but no
    // error will be reported.
    glm::ivec3 cellCoordsAtPoint(const glm::vec3 &point) const
    {
        const AABB box = boundingBox();
        const glm::vec3 mins = box.mins();
        const glm::vec3 p = (point - mins) / (box.extent*2.0f);
        const glm::ivec3 res = gridResolution();
        const glm::ivec3 a(p.x * res.x, p.y * res.y, p.z * res.z);
        return a;
    }
    
    // Gets the center point of the cell in which the specified point resides.
    // Throws an exception if the point is not within this grid.
    glm::vec3 cellCenterAtPoint(const glm::vec3 &point) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(point)) {
                throw OutOfBoundsException();
            }
        }
        
        const glm::vec3 cellDim = cellDimensions();
        const AABB box = boundingBox();
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + box.mins() + (cellDim*0.5f);
        return p;
    }
    
    // Gets the bounding box of the cell in which the specified point resides.
    // Throws an exception if the point is not within this grid.
    AABB cellAtPoint(const glm::vec3 &point) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(point)) {
                throw OutOfBoundsException();
            }
        }
        
        const glm::vec3 cellCenter = cellCenterAtPoint(point);
        const glm::vec3 cellExtent = cellDimensions() * 0.5f;
        const AABB cell = {cellCenter, cellExtent};
        return cell;
    }
    
    // Gets the number of cells along each axis within the specified region.
    // Throws an exception if the region is not within this grid.
    glm::ivec3 countCellsInRegion(const AABB &region) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const glm::ivec3 mins = cellCoordsAtPoint(region.mins());
        const glm::ivec3 maxs = cellCoordsAtPoint(region.maxs());
        const glm::ivec3 size = maxs - mins;
        return size;
    }
    
    // Returns true if the point is within the valid space of the grid.
    bool inbounds(const glm::vec3 &point) const
    {
        const AABB box = boundingBox();
        const glm::vec3 mins = box.mins();
        const glm::vec3 maxs = box.maxs();
        return point.x >= mins.x && point.y >= mins.y && point.z >= mins.z &&
               point.x < maxs.x && point.y < maxs.y && point.z < maxs.z;
    }
    
    // Returns true if the region is within the valid space of the grid.
    bool inbounds(const AABB &region) const
    {
        if (!inbounds(region.mins())) {
            return false;
        } else {
            const glm::vec3 point = region.maxs();
            const glm::vec3 maxs = boundingBox().maxs();
            return point.x <= maxs.x && point.y <= maxs.y && point.z <= maxs.z;
        }
    }
    
    // Iterate over cells in the specified region of the grid.
    // Throws an exception if the region is not within this grid.
    void forEachCell(const AABB &region,
                     std::function<void (const AABB &cell)> fn) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const auto dim = cellDimensions();
        const auto min = region.mins();
        const auto max = region.maxs();
        
        for (glm::vec3 cursor = min; cursor.z < max.z; cursor.z += dim.z) {
            for (cursor.x = min.x; cursor.x < max.x; cursor.x += dim.x) {
                for (cursor.y = min.y; cursor.y < max.y; cursor.y += dim.y) {
                    fn(cellAtPoint(cursor));
                }
            }
        }
    }
    
    // Iterate over cells which fall within the specified frustum.
    inline void forEachCell(const Frustum &frustum, std::function<void (const AABB &cell)> fn) const
    {
        const glm::ivec3 res = gridResolution();
        assert((res.x == res.y) && (res.x == res.z));
        assert(isPowerOfTwo(res.x));
        
        forEachCell(0, ilog2(res.x), boundingBox(), frustum, fn);
    }
    
    // Iterate over cells which fall within the specified frustum.
    void forEachCell(size_t depth,
                     size_t depthOfLeaves,
                     const AABB &box,
                     const Frustum &frustum,
                     std::function<void (const AABB &cell)> fn) const
    {
        if (frustum.boxIsInside(box)) {
            if (depth == depthOfLeaves) {
                fn(box);
            } else {
                for (auto &octant : box.octants()) {
                    forEachCell(depth+1, depthOfLeaves, octant, frustum, fn);
                }
            }
        }
    }
    
    // Iterate over evenly spaced points in the specified region of the grid.
    // Throws an exception if the region is not within this grid.
    void forPointsInGrid(const AABB &region,
                         std::function<void (const glm::vec3 &point)> fn) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const auto dim = cellDimensions();
        const auto min = region.mins();
        const auto max = region.maxs();
        
        for (glm::vec3 cursor = min; cursor.z <= max.z; cursor.z += dim.z) {
            for (cursor.x = min.x; cursor.x <= max.x; cursor.x += dim.x) {
                for (cursor.y = min.y; cursor.y <= max.y; cursor.y += dim.y) {
                    fn(cursor);
                }
            }
        }
    }
};

template<typename TYPE> class GridMutable : public GridAddressable<TYPE>
{
public:
    using GridAddressable<TYPE>::EnableVerboseBoundsChecking;
    using GridAddressable<TYPE>::inbounds;
    using GridAddressable<TYPE>::cellDimensions;
    using GridAddressable<TYPE>::cellAtPoint;
    
    // Get the (mutable) object corresponding to the specified point in space.
    // Throws an exception if the point is not within this grid.
    virtual TYPE& mutableReference(const glm::vec3 &p) = 0;
    
    // Sets the object corresponding to the specified point in space.
    // Note that each point in space corresponds to exactly one cell.
    // Throws an exception if the point is not within this grid.
    virtual void set(const glm::vec3 &p, const TYPE &object) = 0;
    
    // Serially iterate over cells in the specified sub-region of the box.
    // Throws an exception if the region is not within this grid.
    // `fn' paramter is the bounding box of the cell.
    // `fn' returns the new value for the specified cell.
    void mutableForEachCell(const AABB &region,
                            std::function<TYPE (const AABB &cell)> fn)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const auto dim = cellDimensions();
        const auto min = region.mins();
        const auto max = region.maxs();
        
        for (glm::vec3 cursor = min; cursor.z < max.z; cursor.z += dim.z) {
            for (cursor.x = min.x; cursor.x < max.x; cursor.x += dim.x) {
                for (cursor.y = min.y; cursor.y < max.y; cursor.y += dim.y) {
                    const auto cell = cellAtPoint(cursor);
                    const TYPE value = fn(cell);
                    set(cell.center, value);
                }
            }
        }
    }
};

#endif /* GridAddressable_hpp */
