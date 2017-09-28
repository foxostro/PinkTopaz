//
//  SparseGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/27/17.
//
//

#ifndef SparseGrid_hpp
#define SparseGrid_hpp

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"

#include <atomic>
#include <mutex>

// SparseGrid divides space into a regular grid of cells where each cell is
// associated with an element. These elements are accessed atomically by value
// in order to avoid data races. As such, the ElementType must be some type
// compatible with the atomic_load() and atomic_store() functions of the
// standard library, e.g., shared_ptr<T> or atomic<T>.
//
// TODO: Allow multiple simultaneous readers and writers.
// TODO: The grid is sparse and elements only consume storage after they are accessed.
// TODO: The grid may limit it's size and choose to evict items which have not been
//       used recently.
template<typename ElementType>
class SparseGrid : public GridIndexer
{
public:
    ~SparseGrid() = default;
    SparseGrid() = delete;
    
    SparseGrid(const AABB &boundingBox, const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution),
       _array(boundingBox, gridResolution)
    {}
    
    template<typename CoordType>
    ElementType get(const CoordType &p) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const ElementType *src = &_array.reference(p);
        return std::atomic_load(src);
    }
    
    template<typename CoordType>
    void set(const CoordType &p, const ElementType &el)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        ElementType *dst = &_array.mutableReference(p);
        std::atomic_store(dst, el);
    }
    
private:
    mutable std::mutex _mutex;
    Array3D<ElementType> _array;
};

#endif /* SparseGrid_hpp */
