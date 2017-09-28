//
//  ConcurrentGridMutable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/19/17.
//
//

#ifndef ConcurrentGridMutable_hpp
#define ConcurrentGridMutable_hpp

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"

#include <mutex>
#include <functional>
#include <boost/signals2.hpp>
#include <glm/glm.hpp>

// Wraps an Array3D and provides safe concurrent access to the contents.
// Elements of the array are locked independently, allowing multiple readers
// and writers for non-overlapping regions of space.
// ElementType -- The type of the underlying elements of the grid.
template<typename ElementType>
class ConcurrentGridMutable : public GridIndexer
{
public:
    using Reader = std::function<void(const Array3D<ElementType> &data)>;
    using Writer = std::function<void(Array3D<ElementType> &data)>;
    
    // Default Destructor
    ~ConcurrentGridMutable() = default;
    
    // No default constructor.
    ConcurrentGridMutable() = delete;
    
    // Constructor. The space is divided into a grid of cells where each cell
    // is associated with a data element. The space is also divided into a grid
    // of cells where each cell has a lock to protect the data grid from
    // concurrent access. Though, the two grids may have a different resolution
    // such that each lock protects a region of space instead of a single data
    // element.
    // array -- The array to wrap.
    // box -- The region of space for which this grid is valid.
    // lockGridResDivisor -- The resolution of the lock grid is determined by
    //                       dividing the array's grid resolution by this.
    ConcurrentGridMutable(std::unique_ptr<Array3D<ElementType>> &&array,
                          unsigned lockGridResDivisor)
     : GridIndexer(array->boundingBox(), array->gridResolution()),
       _array(std::move(array))
    {}
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    template<typename RegionType>
    void readerTransaction(const RegionType &region, const Reader &fn) const
    {
        auto mutex = _lockArbitrator.getMutex(region);
        std::lock_guard<decltype(mutex)> lock(mutex);
        fn(*_array);
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    template<typename RegionType>
    void writerTransaction(const RegionType &region, const Writer &fn)
    {
        auto mutex = _lockArbitrator.getMutex(region);
        std::lock_guard<decltype(mutex)> lock(mutex);
        fn(*_array);
    }
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region. Syntactic sugar for the common
    // use-case where the client immediately calls forEachCell inside the
    // transaction.
    template<typename RegionType>
    inline void readerTransaction(const RegionType &region,
                                  const std::function<void (const AABB &cell,
                                                            Morton3 index,
                                                            const ElementType &value)> &fn) const
    {
        readerTransaction(region, [&](const Array3D<ElementType> &data){
            data.forEachCell(region, fn);
        });
    }
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. Syntactic sugar for
    // the common use-case where the client immediately calls mutableForEachCell
    // inside the transaction.
    template<typename RegionType>
    inline void writerTransaction(const RegionType &region,
                                  const std::function<void (const AABB &cell,
                                                            Morton3 index,
                                                            ElementType &value)> &fn)
    {
        writerTransaction(region, [&](Array3D<ElementType> &data){
            data.mutableForEachCell(region, fn);
        });
    }
    
    // Gets the array for unprotected, raw access. Use carefully.
    inline const std::unique_ptr<Array3D<ElementType>>& array()
    {
        return _array;
    }
    
protected:
    mutable RegionMutualExclusionArbitrator _lockArbitrator;
    std::unique_ptr<Array3D<ElementType>> _array;
};

#endif /* ConcurrentGridMutable_hpp */
