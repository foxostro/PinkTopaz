//
//  Array3D.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Array3D_hpp
#define Array3D_hpp

#include <glm/vec3.hpp>
#include <vector>

#include "AABB.hpp"
#include "Morton.hpp"
#include "Exception.hpp"
#include "GridIndexer.hpp"

// A regular grid in space where each cell is associated with some object of the
// type specified by `CellType'.
template<typename CellType>
class Array3D : public GridIndexer
{
public:
    using GridIndexer::boundingBox;
    using GridIndexer::gridResolution;
    using GridIndexer::indexAtPoint;
    using GridIndexer::indexAtCellCoords;
    using GridIndexer::cellCoordsAtPoint;
    using GridIndexer::inbounds;
    
    ~Array3D() = default;
    
    // No default constructor.
    Array3D() = delete;
    
    // Constructor. Accepts a bounding box decribing the region of space
    // this grid of objects represents. The space is divided into cells at a
    // resolution described by `resolution.' That is, there are `resolution.x'
    // cells along the X-axis, `resolution.y' cells along the Y-axis, and
    // `resolution.z' cells along the Z-axis.
    Array3D(const AABB &box, glm::ivec3 res)
     : GridIndexer(box, res),
       _maxValidIndex(numberOfInternalElements(res)),
       _cells(numberOfInternalElements(res))
    {}
    
    // Copy constructor.
    Array3D(const Array3D<CellType> &array)
     : GridIndexer(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(numberOfInternalElements(array.gridResolution())),
       _cells(array._cells)
    {}
    
    // Move constructor.
    Array3D(Array3D<CellType> &&array)
     : GridIndexer(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(numberOfInternalElements(array.gridResolution())),
       _cells(std::move(array._cells))
    {}
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    // The bounding box and grid resolution of the two arrays must be the same.
    Array3D<CellType>& operator=(const Array3D<CellType> &array)
    {
        _cells = array._cells;
        return *this;
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    inline const CellType& reference(const glm::vec3 &p) const
    {
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
#endif
        return reference(indexAtPoint(p));
    }
    
    // Get the cell associated with the given cell coordinates.
    inline const CellType& reference(const glm::ivec3 &cellCoords) const
    {
        return reference(indexAtCellCoords(cellCoords));
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    inline const CellType& reference(Morton3 index) const
    {
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(index)) {
            throw OutOfBoundsException();
        }
#endif
        return _cells[(size_t)index];
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    inline CellType& mutableReference(const glm::vec3 &p)
    {
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
#endif
        return mutableReference(indexAtPoint(p));
    }
    
    // Get the (mutable) object associated with the given cell coordinates.
    inline CellType& mutableReference(const glm::ivec3 &cellCoords)
    {
        const Morton3 index = indexAtCellCoords(cellCoords);
        
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(index)) {
            throw OutOfBoundsException();
        }
#endif
        return mutableReference(index);
    }
    
    // Gets the (mutable) object for the specified index, produced by `indexAtPoint'.
    inline CellType& mutableReference(Morton3 index)
    {
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(index)) {
            throw OutOfBoundsException();
        }
#endif
        return _cells[(size_t)index];
    }
    
    // Determine whether a given index is valid.
    inline bool isValidIndex(Morton3 index) const
    {
        return (size_t)index <= _maxValidIndex;
    }
    
private:
    const size_t _maxValidIndex;
    std::vector<CellType> _cells;
    
    // Get the number of elements to use in the internal array.
    static inline size_t numberOfInternalElements(const glm::ivec3 &res)
    {
        return Morton3::encode(res - glm::ivec3(1, 1, 1)) + 1;
    }
};

#endif /* Array3D_hpp */
