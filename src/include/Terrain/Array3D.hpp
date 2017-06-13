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

#define LOG_ARRAY_CTOR 0
#if LOG_ARRAY_CTOR
#include "SDL.h"
#endif

#include <vector>
#include <set>
#include <glm/vec3.hpp>
#include "Exception.hpp"
#include "GridAddressable.hpp"

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class Array3D : public GridMutable<TYPE>
{
public:
    using GridMutable<TYPE>::EnableVerboseBoundsChecking;
    using GridMutable<TYPE>::inbounds;
    using GridMutable<TYPE>::forEachCell;
    using GridMutable<TYPE>::cellCoordsAtPoint;
    using GridMutable<TYPE>::mutableForEachCell;
    
    typedef std::vector<TYPE> container_type;
    typedef typename container_type::size_type index_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    
    ~Array3D()
    {
#if LOG_ARRAY_CTOR
        SDL_Log("~Array3D -- %p", this);
#endif
    }
    
    // No default constructor.
    Array3D() = delete;
    
    // Constructor. Accepts a bounding box decribing the region of space
    // this grid of objects represents. The space is divided into cells at a
    // resolution described by `resolution.' That is, there are `resolution.x'
    // cells along the X-axis, `resolution.y' cells along the Y-axis, and
    // `resolution.z' cells along the Z-axis.
    Array3D(const AABB &box, const glm::ivec3 &res)
     : _cells(res.x * res.y * res.z), _box(box), _res(res)
    {
#if LOG_ARRAY_CTOR
        SDL_Log("Array3D(const AABB &box, const glm::ivec3 &res) -- %p", this);
#endif
    }
    
    // Copy constructor.
    Array3D(const Array3D<TYPE> &array)
     : _cells(array._cells),
       _box(array._box),
       _res(array._res)
    {
#if LOG_ARRAY_CTOR
        SDL_Log("Array3D(const Array3D<TYPE> &array) -- %p", this);
#endif
    }
    
    // Move constructor.
    Array3D(Array3D<TYPE> &&array)
     : _cells(std::move(array._cells)),
       _box(array._box),
       _res(array._res)
    {
#if LOG_ARRAY_CTOR
        SDL_Log("Array3D(Array3D<TYPE> &&array) -- %p", this);
#endif
    }
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    Array3D<TYPE>& operator=(const Array3D<TYPE> &array)
    {
#if LOG_ARRAY_CTOR
        SDL_Log("Array3D<TYPE>& operator=(const Array3D<TYPE> &array) -- %p", this);
#endif
        _cells = array._cells;
        _box = array._box;
        _res = array._res;
        return *this;
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (EnableVerboseBoundsChecking && !inbounds(p)) {
            throw OutOfBoundsException();
        }
        return get(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        if (EnableVerboseBoundsChecking && !inbounds(p)) {
            throw OutOfBoundsException();
        }
        return mutableReference(indexAtPoint(p));
    }
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const TYPE &object) override
    {
        if (EnableVerboseBoundsChecking && !inbounds(p)) {
            throw OutOfBoundsException();
        }
        return set(indexAtPoint(p), object);
    }
    
    // Gets the internal cell index for the specified point in space.
    inline index_type indexAtPoint(const glm::vec3 &point) const
    {
        if (EnableVerboseBoundsChecking && !inbounds(point)) {
            throw OutOfBoundsException();
        }
        
        const glm::ivec3 a = cellCoordsAtPoint(point);
        
        // Columns in the y-axis are contiguous in memory.
        return (a.x * _res.y * _res.z) + (a.z * _res.y) + a.y;
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    inline const TYPE& get(index_type index) const
    {
        return _cells[index];
    }
    
    // Gets the (mutable) object for the specified index, produced by `indexAtPoint'.
    inline TYPE& mutableReference(index_type index)
    {
        return _cells[index];
    }
    
    // Sets the object for the specified index, produced by `indexAtPoint'.
    inline void set(index_type index, const TYPE &object)
    {
        _cells[index] = object;
    }
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    glm::vec3 cellDimensions() const override
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
