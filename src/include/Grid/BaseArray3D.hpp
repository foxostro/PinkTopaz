//
//  Array3D.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef BaseArray3D_hpp
#define BaseArray3D_hpp

#include <glm/vec3.hpp>

#include "AABB.hpp"
#include "Morton.hpp"
#include "Exception.hpp"
#include "GridAddressable.hpp"

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename CellType, typename ContainerType>
class BaseArray3D : public GridMutable<CellType>
{
public:
    using GridMutable<CellType>::indexAtPoint;
    using GridMutable<CellType>::indexAtCellCoords;
    using GridMutable<CellType>::inbounds;
    using GridMutable<CellType>::forEachCell;
    using GridMutable<CellType>::cellCoordsAtPoint;
    using GridMutable<CellType>::mutableForEachCell;
    using GridMutable<CellType>::boundingBox;
    using GridMutable<CellType>::gridResolution;
    
    ~BaseArray3D() = default;
    
    // No default constructor.
    BaseArray3D() = delete;
    
    // Constructor. Accepts a bounding box decribing the region of space
    // this grid of objects represents. The space is divided into cells at a
    // resolution described by `resolution.' That is, there are `resolution.x'
    // cells along the X-axis, `resolution.y' cells along the Y-axis, and
    // `resolution.z' cells along the Z-axis.
    BaseArray3D(const AABB &box, const glm::ivec3 &res)
     : GridMutable<CellType>(box, res),
       _maxValidIndex(numberOfInternalElements(res)),
       _cells(_maxValidIndex)
    {}
    
    // Copy constructor.
    BaseArray3D(const BaseArray3D<CellType, ContainerType> &array)
     : GridMutable<CellType>(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(numberOfInternalElements(array.gridResolution())),
       _cells(array._cells)
    {}
    
    // Move constructor.
    BaseArray3D(BaseArray3D<CellType, ContainerType> &&array)
     : GridMutable<CellType>(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(numberOfInternalElements(array.gridResolution())),
       _cells(std::move(array._cells))
    {}
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    // The bounding box and grid resolution of the two arrays must be the same.
    BaseArray3D<CellType, ContainerType>& operator=(const BaseArray3D<CellType, ContainerType> &array)
    {
        _cells = array._cells;
        return *this;
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    const CellType& get(const glm::vec3 &p) const override
    {
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
#endif
        return get(indexAtPoint(p));
    }
    
    // Get the cell associated with the given cell coordinates.
    const CellType& get(const glm::ivec3 &cellCoords) const override
    {
        return get(indexAtCellCoords(cellCoords));
    }
    
    // Gets the object for the specified index, produced by `indexAtPoint'.
    const CellType& get(Morton3 index) const override
    {
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(index)) {
            throw OutOfBoundsException();
        }
#endif
        return _cells[(size_t)index];
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    CellType& mutableReference(const glm::vec3 &p) override
    {
#ifdef EnableVerboseBoundsChecking
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
#endif
        return mutableReference(indexAtPoint(p));
    }
    
    // Get the (mutable) object associated with the given cell coordinates.
    CellType& mutableReference(const glm::ivec3 &cellCoords) override
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
    CellType& mutableReference(Morton3 index) override
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
    
protected:
    const size_t _maxValidIndex;
    ContainerType _cells;
    
    // Get the number of elements to use in the internal array.
    static inline size_t numberOfInternalElements(const glm::ivec3 &res)
    {
        return Morton3::encode(res - glm::ivec3(1, 1, 1)) + 1;
    }
};

#endif /* BaseArray3D_hpp */
