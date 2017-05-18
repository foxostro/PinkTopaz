//
//  Array3D.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Array3D_hpp
#define Array3D_hpp

#include "AABB.hpp"

#include <vector>
#include <set>
#include <glm/vec3.hpp>
#include "Exception.hpp"

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class Array3D
{
public:
    typedef std::vector<TYPE> container_type;
    typedef typename container_type::size_type index_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    
    // Constructor. Accepts a bounding box decribing the region of space
    // this grid of objects represents. The space is divided into cells at a
    // resolution described by `resolution.' That is, there are `resolution.x'
    // cells along the X-axis, `resolution.y' cells along the Y-axis, and
    // `resolution.z' cells along the Z-axis.
    Array3D(const AABB &box, const glm::ivec3 &res)
     : _cells(res.x * res.y * res.z), _box(box), _res(res)
    {}
    
    // No default constructor.
    Array3D() = delete;
    
    // Copy constructor is just the default.
    Array3D(const Array3D<TYPE> &array) = default;
    
    // Move constructor is just the default.
    Array3D(Array3D<TYPE> &&array) = default;
    
    // Destructor is just the default.
    ~Array3D() = default;
    
    // Each point in space corresponds to exactly one cell. Get the object.
    const TYPE& get(const glm::vec3 &p) const
    {
        assert(inbounds(p));
        return get(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    // If the point is not in bounds then return the specified default value.
    const TYPE& get(const glm::vec3 &p, const TYPE &defaultValue) const
    {
        if (inbounds(p)) {
            return get(indexAtPoint(p));
        } else {
            return defaultValue;
        }
    }
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const TYPE &object)
    {
        assert(inbounds(p));
        return set(indexAtPoint(p), object);
    }
    
    // Returns true if the point is within the valid space of the array.
    inline bool inbounds(const glm::vec3 &point) const
    {
        const glm::vec3 mins = _box.center - _box.extent;
        const glm::vec3 maxs = _box.center + _box.extent;
        return point.x >= mins.x && point.y >= mins.y && point.z >= mins.z &&
               point.x < maxs.x && point.y < maxs.y && point.z < maxs.z;
    }
    
    // Gets the coordinates of the cell in which the specified point resides.
    // These integer coordinates can be used to locate the cell within the grid.
    glm::ivec3 cellCoordsAtPoint(const glm::vec3 &point) const
    {
        assert(inbounds(point));
        const glm::vec3 mins = _box.center - _box.extent;
        const glm::vec3 p = (point - mins) / (_box.extent*2.0f);
        const glm::ivec3 a(p.x * _res.x, p.y * _res.y, p.z * _res.z);
        return a;
    }
    
    // Gets the center point of the cell in which the specified point resides.
    glm::vec3 cellCenterAtPoint(const glm::vec3 &point) const
    {
        assert(inbounds(point));
        const glm::vec3 cellDim = getCellDimensions();
        const glm::ivec3 a = cellCoordsAtPoint(point);
        const glm::vec3 q(a.x * cellDim.x, a.y * cellDim.y, a.z * cellDim.z);
        const glm::vec3 p = q + (_box.center - _box.extent) + (cellDim*0.5f);
        return p;
    }
    
    // Gets the bouding box of the cell in which the specified point resides.
    AABB cellAtPoint(const glm::vec3 &point) const
    {
        assert(inbounds(point));
        const glm::vec3 cellCenter = cellCenterAtPoint(point);
        const glm::vec3 cellExtent = getCellDimensions() * 0.5f;
        const AABB cell = {cellCenter, cellExtent};
        return cell;
    }
    
    // Gets the internal cell index for the specified point in space.
    index_type indexAtPoint(const glm::vec3 &point) const
    {
        assert(inbounds(point));
        
        const glm::ivec3 a = cellCoordsAtPoint(point);
        
        // Columns in the y-axis are contiguous in memory.
        return (a.x * _res.y * _res.z) + (a.z * _res.y) + a.y;
    }
    
    // Gets the set of all indices corresponding to the specified region.
    // Returns a set of pairs where each pair has the index and the AABB of the
    // affected cell.
    auto indicesOverRegion(const AABB &region) const
    {
        std::set<std::pair<index_type, AABB>> indices;
        
        forEach(region, [&](index_type index, const AABB &cell){
            indices.insert(std::make_pair(index, cell));
        });
        
        return indices;
    }
    
    // Iterate over cells in the specified sub-region of the box.
    void forEach(const AABB &region,
                 std::function<void (index_type index,
                                     const AABB &cell)> fn) const
    {
        const auto dim = getCellDimensions();
        const auto min = region.center - region.extent;
        const auto max = region.center + region.extent;
        
        for (glm::vec3 cursor = min; cursor.z < max.z; cursor.z += dim.z) {
            for (cursor.x = min.x; cursor.x < max.x; cursor.x += dim.x) {
                for (cursor.t = min.y; cursor.y < max.y; cursor.y += dim.y) {
                    const auto index = indexAtPoint(cursor);
                    const auto cell = cellAtPoint(cursor);
                    fn(index, cell);
                }
            }
        }
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    const TYPE& get(index_type index) const
    {
        return _cells[index];
    }
    
    // Sets the object for the specified index, produced by `indexAtPoint'.
    void set(index_type index, const TYPE &object)
    {
        _cells[index] = object;
    }
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    inline glm::vec3 getCellDimensions() const
    {
        return glm::vec3(_box.extent.x * 2.0f / _res.x,
                         _box.extent.y * 2.0f / _res.y,
                         _box.extent.z * 2.0f / _res.z);
    }
    
    inline const AABB& getBoundingBox() const { return _box; }
    
    inline const glm::ivec3& getResolution() const { return _res; }
    
    inline iterator begin() { return _cells.begin(); }
    inline const_iterator begin() const { return _cells.begin(); }
    inline iterator end() { return _cells.end(); }
    inline const_iterator end() const { return _cells.end(); }
    
private:
    container_type _cells;
    const AABB _box;
    const glm::ivec3 _res;
};

#endif /* Array3D_hpp */
