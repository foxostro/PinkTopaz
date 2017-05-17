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
#include <glm/vec3.hpp>

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class Array3D
{
public:
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
        return get(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const TYPE &object)
    {
        return set(indexAtPoint(p), object);
    }
    
    // Gets the internal cell index for the specified point in space.
    size_t indexAtPoint(const glm::vec3 &point) const
    {
        const glm::vec3 mins = _box.center - _box.extent;
        const glm::vec3 p = (point - mins) / (_box.extent*2.0f);
        const glm::ivec3 a(p.x * _res.x, p.y * _res.y, p.z * _res.z);
        
        // Columns in the y-axis are contiguous in memory.
        return (a.x * _res.y * _res.z) + (a.z * _res.y) + a.y;
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    const TYPE& get(size_t index) const
    {
        return _cells[index];
    }
    
    // Sets the object for the specified index, produced by `indexAtPoint'.
    void set(size_t index, const TYPE &object)
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
    
private:
    std::vector<TYPE> _cells;
    const AABB _box;
    const glm::ivec3 _res;
};

#endif /* Array3D_hpp */
