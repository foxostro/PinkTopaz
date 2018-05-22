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
       _maxValidIndex(Morton3::encode(res)),
       _cells(numberOfInternalElements(res))
    {}
    
    // Copy constructor.
    Array3D(const Array3D<CellType> &array)
     : GridIndexer(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(Morton3::encode(array.gridResolution())),
       _cells(array._cells)
    {}
    
    // Move constructor.
    Array3D(Array3D<CellType> &&array)
     : GridIndexer(array.boundingBox(), array.gridResolution()),
       _maxValidIndex(Morton3::encode(array.gridResolution())),
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
    
    bool operator==(const Array3D<CellType> &other) const
    {
        if (&other == this) {
            return true;
        }
        
        return boundingBox() == other.boundingBox() &&
               gridResolution() == other.gridResolution() &&
               _cells == other._cells;
    }
    
    // Each point in space corresponds to exactly one cell. Get the object.
    inline const CellType& reference(const glm::vec3 &p) const
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException -- boundingBox={} ; p={}",
                                                       boundingBox(),
                                                       glm::to_string(p)));
            }
        }
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
        if constexpr (EnableVerboseBoundsChecking) {
            if (!isValidIndex(index)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException -- boundingBox={} ; index={} ; maxValidIndex={}",
                                                       boundingBox(),
                                                       (size_t)index,
                                                       _maxValidIndex));
            }
        }
        return _cells[(size_t)index];
    }
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    inline CellType& mutableReference(const glm::vec3 &p)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException -- boundingBox={} ; p={}",
                                                       boundingBox(),
                                                       glm::to_string(p)));
            }
        }
        return mutableReference(indexAtPoint(p));
    }
    
    // Get the (mutable) object associated with the given cell coordinates.
    inline CellType& mutableReference(const glm::ivec3 &cellCoords)
    {
        const Morton3 index = indexAtCellCoords(cellCoords);
        
        if constexpr (EnableVerboseBoundsChecking) {
            if (!isValidIndex(index)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException:\nboundingBox={}\ncellCoords={}\ngridResolution={}\nindex={}\nmaxValidIndex={}",
                                                       boundingBox(),
                                                       glm::to_string(cellCoords),
                                                       glm::to_string(gridResolution()),
                                                       (size_t)index,
                                                       _maxValidIndex));
            }
        }
        return mutableReference(index);
    }
    
    // Gets the (mutable) object for the specified index, produced by `indexAtPoint'.
    inline CellType& mutableReference(Morton3 index)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!isValidIndex(index)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException\nboundingBox={}\nindex={}\nmaxValidIndex={}\nindex corresponds to {}\ngridResolution is {}\nmaxValidIndex corresponds to {}\n_cells.size() = {}",
                                                       boundingBox(),
                                                       (size_t)index,
                                                       _maxValidIndex,
                                                       glm::to_string(index.decode()),
                                                       glm::to_string(gridResolution()),
                                                       glm::to_string(Morton3(gridResolution() - glm::ivec3(1, 1, 1)).decode()),
                                                       _cells.size()));
            }
        }
        return _cells[(size_t)index];
    }
    
    // Determine whether a given index is valid.
    inline bool isValidIndex(Morton3 index) const
    {
        return (size_t)index <= _maxValidIndex;
    }
    
    // Returns a pointer to the raw data. Useful for serialization.
    void* data()
    {
        return (void *)&_cells[0];
    }
    
    // Returns a pointer to the raw data. Useful for serialization.
    const void* data() const
    {
        return (const void *)&_cells[0];
    }
    
private:
    const size_t _maxValidIndex;
    std::vector<CellType> _cells;
    
    // Get the number of elements to use in the internal array.
    static inline size_t numberOfInternalElements(const glm::ivec3 &res)
    {
        return Morton3::encode(res) + 1;
    }
};

#endif /* Array3D_hpp */
