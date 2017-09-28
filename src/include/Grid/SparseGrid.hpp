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
#include <unordered_map>

// SparseGrid divides space into a regular grid of cells where each cell is
// associated with an element. Access to elements is synchronized to avoid
// data races and make this class thread-safe.
//
// TODO: The grid may limit it's size and choose to evict items which have not been
//       used recently.
template<typename ElementType>
class SparseGrid : public GridIndexer
{
public:
    ~SparseGrid() = default;
    SparseGrid() = delete;
    
    SparseGrid(const AABB &boundingBox, const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution)
    {}
    
    inline ElementType get(const Morton3 &morton)
    {
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(morton)) {
            throw OutOfBoundsException();
        }
#endif
        std::lock_guard<std::mutex> lock(_mutex);
        return _hashMap[(size_t)morton];
    }
    
    inline void set(const Morton3 &morton, const ElementType &el)
    {
#ifdef EnableVerboseBoundsChecking
        if (!isValidIndex(morton)) {
            throw OutOfBoundsException();
        }
#endif
        std::lock_guard<std::mutex> lock(_mutex);
        _hashMap[(size_t)morton] = el;
    }
    
    inline ElementType get(const glm::vec3 &p)
    {
#ifdef EnableVerboseBoundsChecking
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
#endif
        return get(indexAtPoint(p));
    }
    
    inline void set(const glm::vec3 &p, const ElementType &el)
    {
#ifdef EnableVerboseBoundsChecking
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
#endif
        return set(indexAtPoint(p), el);
    }
    
private:
    std::mutex _mutex;
    std::unordered_map<size_t, ElementType> _hashMap;
};

#endif /* SparseGrid_hpp */
