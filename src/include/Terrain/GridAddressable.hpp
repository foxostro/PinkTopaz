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
#include "Morton.hpp"
#include "ChangeLog.hpp"
#include <functional>

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
isPointInsideBox(Morton3 index,
                 const glm::ivec3 &mins,
                 const glm::ivec3 &maxs)
{
    return isPointInsideBox(index.decode(), mins, maxs);
}

static inline bool
isPointInsideBox(const glm::vec3 &point, const AABB &box)
{
    return isPointInsideBox(point, box.mins(), box.maxs());
}

// Exception thrown when attempting to access the grid at a point that is not in
// the valid space of the grid.
class OutOfBoundsException : public Exception
{
public:
    OutOfBoundsException() : Exception("out of bounds") {}
};

// A GridAddressable is a regular grid of objects in space.
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
    
    // Get the cell associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    virtual const TYPE& get(const glm::ivec3 &cellCoords) const = 0;
    
    // Get the cell associated with the given morton code.
    // Morton codes can be used to uniquely identify a cell in the grid. At the
    // very least, this code can be used to encode cell coordinates. Sub-classes
    // of GridAddressable may override this method to allow the code to be used
    // to directly index some underlying grid array.
    virtual const TYPE& get(Morton3 index) const
    {
        return get(index.decode());
    }
    
    // Gets a morton code to identify the cell for the specified point in space.
    inline Morton3 indexAtPoint(const glm::vec3 &point) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(point)) {
                throw OutOfBoundsException();
            }
        }
        
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const Morton3 index = indexAtCellCoords(a);
        return index;
    }
    
    // Gets a morton code to identify the cell for the specified grid cell.
    inline Morton3 indexAtCellCoords(const glm::ivec3 &cellCoords) const
    {
        return Morton3(cellCoords);
    }
    
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
    // This method does not check that `point' actually resides in the
    // valid space of the grid.
    inline glm::ivec3 cellCoordsAtPoint(const glm::vec3 &point) const
    {
        const AABB box = boundingBox();
        const glm::vec3 mins = box.mins();
        const glm::vec3 p = (point - mins) / (box.extent*2.0f);
        const glm::ivec3 res = gridResolution();
        const glm::ivec3 a(p.x * res.x, p.y * res.y, p.z * res.z);
        return a;
    }
    
    // Gets the coordinates of the cell in which the specified point resides.
    // These integer coordinates can be used to locate the cell within the grid.
    // Notably, make sure to round up to the next cell when the specified point
    // is on the maximum edge of the cell.
    //
    // This method does not check that `point' actually resides in the
    // valid space of the grid.
    inline glm::ivec3 cellCoordsAtPointRoundUp(const glm::vec3 &point) const
    {
        const AABB box = boundingBox();
        const glm::vec3 mins = box.mins();
        const glm::vec3 p = (point - mins) / (box.extent*2.0f);
        const glm::ivec3 res = gridResolution();
        const glm::ivec3 a(ceilf(p.x * res.x),
                           ceilf(p.y * res.y),
                           ceilf(p.z * res.z));
        return a;
    }
    
    // Gets the center point of the cell at the specified cell coordinates.
    //
    // This method does not check that cell coords `a' actually resides in the
    // valid space of the grid.
    inline glm::vec3 cellCenterAtCellCoords(const glm::ivec3 &a) const
    {
        const glm::vec3 cellDim = cellDimensions();
        const glm::vec3 cellExtent = cellDim * 0.5f;
        const AABB box = boundingBox();
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + box.mins() + cellExtent;
        return p;
    }
    
    // Gets the center point of the cell in which the specified point resides.
    //
    // This method does not check that `point' actually resides in the
    // valid space of the grid.
    inline glm::vec3 cellCenterAtPoint(const glm::vec3 &point) const
    {
        const glm::vec3 cellDim = cellDimensions();
        const AABB box = boundingBox();
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + box.mins() + (cellDim*0.5f);
        return p;
    }
    
    // Gets the bounding box of the cell in which the specified point resides.
    //
    // This method does not check that `point' actually resides in the
    // valid space of the grid.
    inline AABB cellAtPoint(const glm::vec3 &point) const
    {
        const glm::vec3 cellCenter = cellCenterAtPoint(point);
        const glm::vec3 cellExtent = cellDimensions() * 0.5f;
        const AABB cell = {cellCenter, cellExtent};
        return cell;
    }
    
    // Gets the number of cells along each axis within the specified region.
    //
    // This method does not check that `region' actually resides in the
    // valid space of the grid.
    inline glm::ivec3 countCellsInRegion(const AABB &region) const
    {
        const glm::ivec3 mins = cellCoordsAtPoint(region.mins());
        const glm::ivec3 maxs = cellCoordsAtPoint(region.maxs());
        const glm::ivec3 size = maxs - mins;
        return size;
    }
    
    // Adjusts the specified box so that it includes the space for all cells
    // which fall within that region.
    //
    // This method does not check that `cellCoords' actually resides in the
    // valid space of the grid.
    inline AABB snapRegionToCellBoundaries(const AABB &region) const
    {
        // Zero size boxes are a special case. Return a zero size box at the
        // center of the cell on which region.center resides.
        static const glm::vec3 zero(0.f, 0.f, 0.f);
        if (region.extent == zero) {
            return {cellCenterAtPoint(region.center), zero};
        }
        
        const auto minCellCoords = cellCoordsAtPoint(region.mins());
        const auto maxCellCoords = cellCoordsAtPointRoundUp(region.maxs()) - glm::ivec3(1, 1, 1);
        
        const auto cellExtent = cellDimensions() * 0.5f;
        const AABB minCell = {cellCenterAtCellCoords(minCellCoords), cellExtent};
        const AABB maxCell = {cellCenterAtCellCoords(maxCellCoords), cellExtent};
        
        const auto minCorner = minCell.mins();
        const auto maxCorner = maxCell.maxs();
        
        const auto center = (maxCorner + minCorner) * 0.5f;
        const auto extent = (maxCorner - minCorner) * 0.5f;
        const AABB adjustedRegion = {center, extent};
        
        return adjustedRegion;
    }
    
    // Returns true if the point is within the valid space of the grid.
    inline bool inbounds(const glm::vec3 &point) const
    {
        return isPointInsideBox(point, boundingBox());
    }
    
    // Returns true if the point is within the valid space of the grid.
    inline bool inbounds(const glm::ivec3 &a) const
    {
        return isPointInsideBox(a, glm::ivec3(0, 0, 0), gridResolution());
    }
    
    // Returns true if the point is within the valid space of the grid.
    inline bool inbounds(Morton3 index) const
    {
        return inbounds(index.decode());
    }
    
    // Returns true if the region is within the valid space of the grid.
    inline bool inbounds(const AABB &region) const
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
                     const std::function<void (const AABB &cell,
                                               Morton3 index)> &fn) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const auto dim = cellDimensions();
        const auto extent = dim * 0.5f;
        const auto min = region.mins();
        const auto max = region.maxs();
        const auto minCellCoords = cellCoordsAtPoint(min);
        const auto maxCellCoords = cellCoordsAtPointRoundUp(max);
        
#ifndef NDEBUG
        assert(inbounds(minCellCoords));
        assert(minCellCoords.x <= maxCellCoords.x);
        assert(minCellCoords.y <= maxCellCoords.y);
        assert(minCellCoords.z <= maxCellCoords.z);
#endif
        
        for (glm::ivec3 cellCoords = minCellCoords; cellCoords.z < maxCellCoords.z; ++cellCoords.z) {
            for (cellCoords.x = minCellCoords.x; cellCoords.x < maxCellCoords.x; ++cellCoords.x) {
                for (cellCoords.y = minCellCoords.y; cellCoords.y < maxCellCoords.y; ++cellCoords.y) {
                    const glm::vec3 center = cellCenterAtCellCoords(cellCoords);
                    const auto index = indexAtCellCoords(cellCoords);
                    fn({center, extent}, index);
                }
            }
        }
    }
    
    // Iterate over cells in the specified region of the grid.
    // Throws an exception if the region is not within this grid.
    void forEachCell(const AABB &region,
                     const std::function<void (const glm::ivec3 &a)> &fn) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
        }
        
        const auto min = region.mins();
        const auto max = region.maxs();
        const auto minCellCoords = cellCoordsAtPoint(min);
        const auto maxCellCoords = cellCoordsAtPointRoundUp(max);
        
#ifndef NDEBUG
        assert(inbounds(minCellCoords));
        assert(minCellCoords.x <= maxCellCoords.x);
        assert(minCellCoords.y <= maxCellCoords.y);
        assert(minCellCoords.z <= maxCellCoords.z);
#endif
        
        for (glm::ivec3 cellCoords = minCellCoords; cellCoords.z < maxCellCoords.z; ++cellCoords.z) {
            for (cellCoords.x = minCellCoords.x; cellCoords.x < maxCellCoords.x; ++cellCoords.x) {
                for (cellCoords.y = minCellCoords.y; cellCoords.y < maxCellCoords.y; ++cellCoords.y) {
                    fn(cellCoords);
                }
            }
        }
    }
    
    // Iterate over cells in the specified region of the grid.
    // Throws an exception if the region is not within this grid.
    void forEachCell(const AABB &region,
                     const std::function<void (const AABB &cell,
                                               Morton3 index,
                                               const TYPE &value)> &fn) const
    {
        forEachCell(region, [&](const AABB &cell, Morton3 index){
            fn(cell, index, get(index));
        });
    }
    
    // Iterate over cells which fall within the specified frustum.
    inline void forEachCell(const Frustum &frustum,
                            std::function<void (const AABB &cell,
                                                Morton3 index,
                                                const TYPE &value)> fn) const
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
                     std::function<void (const AABB &cell,
                                         Morton3 index,
                                         const TYPE &value)> fn) const
    {
        if (frustum.boxIsInside(box)) {
            if (depth == depthOfLeaves) {
                // AFOX_TODO: I suspect the conversion from `box' to `index'
                // could be accelerated with knowledge of the Z-order layout
                // since it defines a linear octree traversal.
                Morton3 index(cellCoordsAtPoint(box.center));
                fn(box, index, get(index));
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
    
    // Get a box in cell-coordinate space which includes all cells that fall in
    // the specified region. For any cell-coordinate in this box, the specified
    // region will include that cell.
    inline _AABB<int> cellsInRegion(const AABB &region) const
    {
        const auto min = region.mins();
        const auto max = region.maxs();
        const auto minCellCoords = cellCoordsAtPoint(min);
        const auto maxCellCoords = cellCoordsAtPointRoundUp(max);
        
        const auto center = (maxCellCoords + minCellCoords) / 2;
        const auto extent = (maxCellCoords - minCellCoords) / 2;
        
        assert(minCellCoords == (center - extent));
        assert(maxCellCoords == (center + extent));
        
        _AABB<int> cellBox = {center, extent};
        return cellBox;
    }
};

// GridMutable provides API for mutating GridAddressable. It is a mutable grid.
template<typename TYPE> class GridMutable : public GridAddressable<TYPE>
{
public:
    using GridAddressable<TYPE>::EnableVerboseBoundsChecking;
    using GridAddressable<TYPE>::inbounds;
    using GridAddressable<TYPE>::cellDimensions;
    using GridAddressable<TYPE>::cellAtPoint;
    using GridAddressable<TYPE>::cellCoordsAtPoint;
    using GridAddressable<TYPE>::indexAtPoint;
    using GridAddressable<TYPE>::forEachCell;
    
    // Get the (mutable) object corresponding to the specified point in space.
    // Throws an exception if the point is not within this grid.
    virtual TYPE& mutableReference(const glm::vec3 &p) = 0;
    
    // Get the (mutable) object associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    virtual TYPE& mutableReference(const glm::ivec3 &cellCoords) = 0;
    
    // Get the (mutable) object corresponding to the specified morton code.
    // Morton codes can be used to uniquely identify a cell in the grid. At the
    // very least, this code can be used to encode cell coordinates. Sub-classes
    // of GridMutable may override this method to allow the code to be used to
    // directly index some underlying grid array.
    virtual TYPE& mutableReference(Morton3 index)
    {
        glm::ivec3 a = index.decode();
        return mutableReference(a);
    }
    
    // Serially iterate over cells in the specified sub-region of the box.
    // Throws an exception if the region is not within this grid.
    // `fn' parameters are the bounding box of the cell, the cell index, and a
    // mutable reference to the value.
    void mutableForEachCell(const AABB &region,
                            const std::function<void (const AABB &cell,
                                                      Morton3 index,
                                                      TYPE &value)> &fn)
    {
        forEachCell(region, [&](const AABB &cell, Morton3 index){
            fn(cell, index, mutableReference(index));
        });
    }
};

#endif /* GridAddressable_hpp */
