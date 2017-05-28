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
#include "GridAddressable.hpp"

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class Array3D : public GridMutable<TYPE>
{
public:
    using GridAddressable<TYPE>::EnableBoundsChecking;
    using GridAddressable<TYPE>::inbounds;
    using GridAddressable<TYPE>::forEachCell;
    using GridAddressable<TYPE>::cellCoordsAtPoint;
    
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
    
    // Destructor is just the default.
    ~Array3D() = default;
    
    // Each point in space corresponds to exactly one cell. Get the object.
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (EnableBoundsChecking && !inbounds(p)) {
            throw Exception("out of bounds");
        }
        return get(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        if (EnableBoundsChecking && !inbounds(p)) {
            throw Exception("out of bounds");
        }
        return mutableReference(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const TYPE &object) override
    {
        if (EnableBoundsChecking && !inbounds(p)) {
            throw Exception("out of bounds");
        }
        return set(indexAtPoint(p), object);
    }
    
    // Gets the internal cell index for the specified point in space.
    index_type indexAtPoint(const glm::vec3 &point) const
    {
        if (EnableBoundsChecking && !inbounds(point)) {
            throw Exception("out of bounds");
        }
        
        const glm::ivec3 a = cellCoordsAtPoint(point);
        
        // Columns in the y-axis are contiguous in memory.
        return (a.x * _res.y * _res.z) + (a.z * _res.y) + a.y;
    }
    
    // Gets the set of all indices corresponding to the specified region.
    // Returns a set of pairs where each pair has the index and the AABB of the
    // affected cell.
    auto indicesOverRegion(const AABB &region) const
    {
        if (EnableBoundsChecking && !inbounds(region)) {
            throw Exception("out of bounds");
        }
        
        std::set<std::pair<index_type, AABB>> indices;
        
        forEachCell(region, [&](const AABB &cell){
            const auto index = indexAtPoint(cell.center);
            indices.insert(std::make_pair(index, cell));
        });
        
        return indices;
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    const TYPE& get(index_type index) const
    {
        return _cells[index];
    }
    
    // Gets the (mutable) object for the specified index, produced by `indexAtPoint'.
    TYPE& mutableReference(index_type index)
    {
        return _cells[index];
    }
    
    // Sets the object for the specified index, produced by `indexAtPoint'.
    void set(index_type index, const TYPE &object)
    {
        _cells[index] = object;
    }
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    glm::vec3 getCellDimensions() const override
    {
        return glm::vec3(_box.extent.x * 2.0f / _res.x,
                         _box.extent.y * 2.0f / _res.y,
                         _box.extent.z * 2.0f / _res.z);
    }
    
    // Gets the region for which the grid is defined.
    AABB boundingBox() const override { return _box; }
    
    // Gets the number of cells along each axis within the valid region.
    glm::ivec3 gridResolution() const override { return _res; }
    
    inline iterator begin() { return _cells.begin(); }
    inline const_iterator begin() const { return _cells.begin(); }
    inline iterator end() { return _cells.end(); }
    inline const_iterator end() const { return _cells.end(); }
    
private:
    container_type _cells;
    AABB _box;
    glm::ivec3 _res;
};

#endif /* Array3D_hpp */
