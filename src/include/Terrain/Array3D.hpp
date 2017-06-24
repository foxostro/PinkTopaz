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
#include "Morton.hpp"

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
    using GridMutable<TYPE>::indexAtPoint;
    using GridMutable<TYPE>::indexAtCellCoords;
    using GridMutable<TYPE>::inbounds;
    using GridMutable<TYPE>::forEachCell;
    using GridMutable<TYPE>::cellCoordsAtPoint;
    using GridMutable<TYPE>::mutableForEachCell;
    
    using container_type = std::vector<TYPE>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    
    ~Array3D() = default;
    
    // No default constructor.
    Array3D() = delete;
    
    // Constructor. Accepts a bounding box decribing the region of space
    // this grid of objects represents. The space is divided into cells at a
    // resolution described by `resolution.' That is, there are `resolution.x'
    // cells along the X-axis, `resolution.y' cells along the Y-axis, and
    // `resolution.z' cells along the Z-axis.
    Array3D(const AABB &box, const glm::ivec3 &res)
     : GridMutable<TYPE>(box, res),
       _cells(numberOfInternalElements(res)),
       _box(box),
       _res(res)
    {}
    
    // Copy constructor.
    Array3D(const Array3D<TYPE> &array)
     : _cells(array._cells),
       _box(array._box),
       _res(array._res)
    {}
    
    // Move constructor.
    Array3D(Array3D<TYPE> &&array)
     : GridMutable<TYPE>(array._box, array._res),
        _cells(std::move(array._cells)),
        _box(array._box),
        _res(array._res)
    {}
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    // The bounding box and grid resolution of the two arrays must be the same.
    Array3D<TYPE>& operator=(const Array3D<TYPE> &array)
    {
        assert(_box == array._box);
        assert(_res == array._res);
        _cells = array._cells;
        return *this;
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    const TYPE& get(const glm::vec3 &p) const override
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
        }
        return get(indexAtPoint(p));
    }
    
    // Get the cell associated with the given cell coordinates.
    const TYPE& get(const glm::ivec3 &cellCoords) const override
    {
        return get(indexAtCellCoords(cellCoords));
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    const TYPE& get(Morton3 index) const override
    {
        return _cells[(size_t)index];
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
        }
        return mutableReference(indexAtPoint(p));
    }
    
    // Get the (mutable) object associated with the given cell coordinates.
    TYPE& mutableReference(const glm::ivec3 &cellCoords) override
    {
        return mutableReference(indexAtCellCoords(cellCoords));
    }
    
    // Gets the (mutable) object for the specified index, produced by `indexAtPoint'.
    TYPE& mutableReference(Morton3 index) override
    {
        return _cells[(size_t)index];
    }
    
    // Determine whether a given index is valid.
    inline bool isValidIndex(Morton3 index) const
    {
        return (size_t)index < _cells.size();
    }
    
    inline iterator begin()
    {
        return _cells.begin();
    }
    
    inline const_iterator begin() const
    {
        return _cells.begin();
    }
    
    inline iterator end()
    {
        return _cells.end();
    }
    
    inline const_iterator end() const
    {
        return _cells.end();
    }
    
private:
    container_type _cells;
    const AABB _box;
    const glm::ivec3 _res;
    
    // Get the number of elements to use in the internal array.
    static inline size_t numberOfInternalElements(const glm::ivec3 &res)
    {
        return Morton3::encode(res - glm::ivec3(1, 1, 1)) + 1;
    }
};

#endif /* Array3D_hpp */
