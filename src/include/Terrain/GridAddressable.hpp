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
    // This method will not throw an exception if the point is outside the valid
    // space of the grid. In this case, you will receive garbage results, but no
    // error will be reported.
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
    // This method will not throw an exception if the point is outside the valid
    // space of the grid. In this case, you will receive garbage results, but no
    // error will be reported.
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
    
    // Convert the specified cell coordinates into world-space coordinates.
    // This method provides no error checking for the validity of the input.
    inline const glm::vec3 worldPosAtCellCoords(const glm::ivec3 &cellCoords) const
    {
        const glm::vec3 dim = cellDimensions();
        const glm::vec3 extent = dim * 0.5f;
        const glm::vec3 min = boundingBox().mins() + extent;
        const glm::vec3 worldPos = min + glm::vec3(cellCoords.x * dim.x,
                                                   cellCoords.y * dim.y,
                                                   cellCoords.z * dim.z);
        return worldPos;
    }
    
    // Gets the center point of the cell in which the specified point resides.
    // Throws an exception if the point is not within this grid.
    inline glm::vec3 cellCenterAtPoint(const glm::vec3 &point) const
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
    
    // Gets the center point of the cell at the specified cell coordinates.
    inline glm::vec3 cellCenterAtCellCoords(const glm::ivec3 &a) const
    {
        const glm::vec3 cellDim = cellDimensions();
        const glm::vec3 cellExtent = cellDim * 0.5f;
        const AABB box = boundingBox();
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + box.mins() + cellExtent;
        return p;
    }
    
    // Gets the bounding box of the cell in which the specified point resides.
    // Throws an exception if the point is not within this grid.
    inline AABB cellAtPoint(const glm::vec3 &point) const
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
    inline glm::ivec3 countCellsInRegion(const AABB &region) const
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
    inline bool inbounds(const glm::vec3 &point) const
    {
        const AABB box = boundingBox();
        const glm::vec3 mins = box.mins();
        const glm::vec3 maxs = box.maxs();
        return point.x >= mins.x && point.y >= mins.y && point.z >= mins.z &&
               point.x < maxs.x && point.y < maxs.y && point.z < maxs.z;
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
        const auto res = gridResolution();
        assert(minCellCoords.x >= 0 && minCellCoords.x <= res.x);
        assert(minCellCoords.y >= 0 && minCellCoords.y <= res.y);
        assert(minCellCoords.z >= 0 && minCellCoords.z <= res.z);
        assert(maxCellCoords.x >= 0 && maxCellCoords.x <= res.x);
        assert(maxCellCoords.y >= 0 && maxCellCoords.y <= res.y);
        assert(maxCellCoords.z >= 0 && maxCellCoords.z <= res.z);
        assert(minCellCoords.x <= maxCellCoords.x);
        assert(minCellCoords.y <= maxCellCoords.y);
        assert(minCellCoords.z <= maxCellCoords.z);
#endif
        
        for (glm::ivec3 cellCoords = minCellCoords; cellCoords.z < maxCellCoords.z; ++cellCoords.z) {
            for (cellCoords.x = minCellCoords.x; cellCoords.x < maxCellCoords.x; ++cellCoords.x) {
                for (cellCoords.y = minCellCoords.y; cellCoords.y < maxCellCoords.y; ++cellCoords.y) {
                    const glm::vec3 center = cellCenterAtCellCoords(cellCoords);
                    const Morton3 index(cellCoords);
                    fn({center, extent}, index);
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
    
    // Sets the object corresponding to the specified point in space.
    // Note that each point in space corresponds to exactly one cell.
    // Throws an exception if the point is not within this grid.
    virtual void set(const glm::vec3 &p, const TYPE &object) = 0;
    
    // Sets the cell associated with the given cell coordinates.
    // Each cell in the grid can be addressed by cell coordinates which uniquely
    // identify that cell.
    // See also gridResolution() and cellCoordsAtPoint().
    virtual void set(const glm::ivec3 &cellCoords, const TYPE &object) = 0;
    
    // Sets the object for the specified index, produced by `indexAtPoint'.
    // Morton codes can be used to uniquely identify a cell in the grid. At the
    // very least, this code can be used to encode cell coordinates. Sub-classes
    // of GridMutable may override this method to allow the code to be used to
    // directly index some underlying grid array.
    virtual void set(Morton3 index, const TYPE &object)
    {
        set(index.decode(), object);
    }
    
    // Serially iterate over cells in the specified sub-region of the box.
    // Throws an exception if the region is not within this grid.
    // `fn' paramter is the bounding box of the cell.
    // `fn' returns the new value for the specified cell.
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

// GridTransactional provides wraps a mutable grid, providing access to that
// grid only through atomic transactions.
//
// The simplest implementation would wrap the protect grid in a mutex or a
// shared_mutex (reader/writer lock). More complicated implementations could do
// something to permit concurrent writers by locking portions of the grid.
template <typename TYPE> class GridTransactional
{
public:
    typedef std::function<void(const GridAddressable<TYPE> &data)> Reader;
    typedef std::function<ChangeLog(GridMutable<TYPE> &data)> Writer;
    
    virtual ~GridTransactional() = default;
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying voxel data in the specified region.
    // r -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    virtual void readerTransaction(const AABB &r, const Reader &fn) const = 0;
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    virtual void writerTransaction(const AABB &region, const Writer &fn) = 0;
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    virtual glm::vec3 cellDimensions() const = 0;
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    virtual AABB boundingBox() const = 0;
    
    // Gets the number of cells along each axis within the valid region.
    virtual glm::ivec3 gridResolution() const = 0;
};

#endif /* GridAddressable_hpp */
