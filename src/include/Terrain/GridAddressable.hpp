//
//  GridAddressable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/20/17.
//
//

#ifndef GridAddressable_hpp
#define GridAddressable_hpp

#include <glm/vec3.hpp>

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class GridAddressable
{
public:
    static constexpr bool EnableVerboseBoundsChecking = false;
    
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
    glm::vec3 cellCenterAtPoint(const glm::vec3 &point) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(point)) {
            throw Exception("out of bounds");
        }
        
        const glm::vec3 cellDim = cellDimensions();
        const AABB box = boundingBox();
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + box.mins() + (cellDim*0.5f);
        return p;
    }
    
    // Gets the bouding box of the cell in which the specified point resides.
    AABB cellAtPoint(const glm::vec3 &point) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(point)) {
            throw Exception("out of bounds");
        }
        
        const glm::vec3 cellCenter = cellCenterAtPoint(point);
        const glm::vec3 cellExtent = cellDimensions() * 0.5f;
        const AABB cell = {cellCenter, cellExtent};
        return cell;
    }
    
    // Gets the number of cells along each axis within the specified region.
    glm::ivec3 countCellsInRegion(const AABB &region) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(region)) {
            throw Exception("out of bounds");
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
    void forEachCell(const AABB &region,
                     std::function<void (const AABB &cell)> fn) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(region)) {
            throw Exception("out of bounds");
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
    
    // Iterate over evenly spaced points in the specified region of the grid.
    void forPointsInGrid(const AABB &region,
                         std::function<void (const glm::vec3 &point)> fn) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(region)) {
            throw Exception("out of bounds");
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
    // `fn' paramter is the bounding box of the cell.
    // `fn' returns the new value for the specified cell.
    void mutableForEachCell(const AABB &region,
                            std::function<TYPE (const AABB &cell)> fn)
    {
        if (EnableVerboseBoundsChecking && !inbounds(region)) {
            throw Exception("out of bounds");
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
