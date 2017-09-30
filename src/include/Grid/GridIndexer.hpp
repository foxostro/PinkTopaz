//
//  GridIndexer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/20/17.
//
//

#ifndef GridIndexer_hpp
#define GridIndexer_hpp

#include "AABB.hpp"
#include "Exception.hpp"
#include "Frustum.hpp"
#include "Morton.hpp"
#include <functional>


#ifndef NDEBUG
#define EnableVerboseBoundsChecking
#endif


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


// Exception thrown when attempting to access the grid at a point that is not in
// the valid space of the grid.
class OutOfBoundsException : public Exception
{
public:
    OutOfBoundsException() : Exception("out of bounds") {}
};


// A "Grid" is an object that divides space into a regular grid with equal sized
// cells where each cell is associated with an object. The GridIndexer deals
// with mapping from 3D world space to a 3D cell coordinate space and from cell
// coordinate space to the 1D index space. This is all entirely independent
// of the actual storage and retrieval of elements in a grid.
class GridIndexer
{
public:
    class Iterator
    {
    public:
        Iterator() = delete;
        
        Iterator(glm::ivec3 min, glm::ivec3 max, glm::ivec3 pos)
         : _min(min), _max(max), _pos(pos)
        {}
        
        Iterator(const Iterator &a) = default;
        
        Iterator& operator++()
        {
            ++_pos.y;
            if (_pos.y >= _max.y) {
                _pos.y = _min.y;
                
                ++_pos.x;
                if (_pos.x >= _max.x) {
                    _pos.x = _min.x;
                    
                    ++_pos.z;
                    if (_pos.z >= _max.z) {
                        // sentinel value for the end of the sequence
                        _pos = _max;
                    }
                }
            }
            
            return *this;
        }
        
        Iterator operator++(int)
        {
            Iterator temp(*this);
            operator++();
            return temp;
        }
        
        glm::ivec3 operator*()
        {
            return _pos;
        }
        
        glm::ivec3* operator->()
        {
            return &_pos;
        }
        
        bool operator==(const Iterator &a) const
        {
            return _min == a._min &&
                   _max == a._max &&
                   _pos == a._pos;
        }
        
        bool operator!=(const Iterator &a) const
        {
            return !(*this == a);
        }
        
    private:
        glm::ivec3 _min, _max, _pos;
    };
    
    template<typename IteratorType>
    class Range
    {
    public:
        Range() = delete;
        Range(IteratorType a, IteratorType b) : _begin(a), _end(b) {}
        
        IteratorType begin() const
        {
            return _begin;
        }
        
        IteratorType end() const
        {
            return _end;
        }
        
    private:
        IteratorType _begin, _end;
    };
    
    virtual ~GridIndexer() = default;
    
    GridIndexer(const AABB &boundingBox,
                const glm::ivec3 &gridResolution)
     : _boundingBox(boundingBox),
       _gridResolution(gridResolution),
       _cellDimensions((boundingBox.extent * 2.0f).x / gridResolution.x,
                       (boundingBox.extent * 2.0f).y / gridResolution.y,
                       (boundingBox.extent * 2.0f).z / gridResolution.z)
    {}
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    inline const AABB& boundingBox() const
    {
        return _boundingBox;
    }
    
    // Gets the number of cells along each axis within the valid region.
    inline const glm::ivec3& gridResolution() const
    {
        return _gridResolution;
    }
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    inline const glm::vec3& cellDimensions() const
    {
        return _cellDimensions;
    }
    
    // Gets a morton code to identify the cell for the specified point in space.
    inline Morton3 indexAtPoint(const glm::vec3 &point) const
    {
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(point)) {
                throw OutOfBoundsException();
            }
#endif
        
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const Morton3 index = indexAtCellCoords(a);
        return index;
    }
    
    // Gets a morton code to identify the cell for the specified grid cell.
    inline Morton3 indexAtCellCoords(const glm::ivec3 &cellCoords) const
    {
        return Morton3(cellCoords);
    }
    
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
    
    // Gets the bounding box of the cell in which the specified point resides.
    //
    // This method does not check that `a' actually resides in the
    // valid space of the grid.
    inline AABB cellAtCellCoords(const glm::ivec3 &a) const
    {
        const glm::vec3 cellCenter = cellCenterAtCellCoords(a);
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
    
    // Return a Range object which can iterate over a sub-region of the grid.
    auto slice(const AABB &region) const
    {
#ifdef EnableVerboseBoundsChecking
        if (!inbounds(region)) {
            throw OutOfBoundsException();
        }
#endif
        
        const auto min = region.mins();
        const auto max = region.maxs();
        const auto minCellCoords = cellCoordsAtPoint(min);
        const auto maxCellCoords = cellCoordsAtPointRoundUp(max);
        
        return Range<Iterator>(Iterator(minCellCoords,
                                        maxCellCoords,
                                        minCellCoords),
                               Iterator(minCellCoords,
                                        maxCellCoords,
                                        maxCellCoords));
    }
    
    // Iterate over cells which fall within the specified frustum.
    // TODO: Replace with iterator for a Frustum and accompanying slice() method
    inline void forEachCell(const Frustum &frustum,
                            const std::function<void (const AABB &cell,
                                                      Morton3 index)> &fn) const
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
                     const std::function<void (const AABB &cell,
                                               Morton3 index)> &fn) const
    {
        if (frustum.boxIsInside(box)) {
            if (depth == depthOfLeaves) {
                // AFOX_TODO: I suspect the conversion from `box' to `index'
                // could be accelerated with knowledge of the Z-order layout
                // since it defines a linear octree traversal.
                auto index = indexAtPoint(box.center);
                fn(box, index);
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
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(region)) {
                throw OutOfBoundsException();
            }
#endif
        
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
    
private:
    const AABB _boundingBox;
    const glm::ivec3 _gridResolution;
    const glm::vec3 _cellDimensions;
};

#endif /* GridIndexer_hpp */
