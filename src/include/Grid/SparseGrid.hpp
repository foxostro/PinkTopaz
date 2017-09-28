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
#include "Grid/GridLRU.hpp"
#include <unordered_map>

// SparseGrid divides space into a regular grid of cells where each cell is
// associated with an element. Access to elements is synchronized to avoid
// data races and make this class thread-safe.
//
// The grid may limit it's size and choose to evict items which have not been
// used recently.
template<typename ElementType>
class SparseGrid : public GridIndexer
{
public:
    ~SparseGrid() = default;
    SparseGrid() = delete;
    
    SparseGrid(const AABB &boundingBox, const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution),
       _countLimit(std::numeric_limits<std::size_t>::max())
    {}
    
    inline ElementType get(const Morton3 &morton)
    {
        return get((size_t)morton);
    }
    
    inline void set(const Morton3 &morton, const ElementType &el)
    {
        return set((size_t)morton, el);
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
    
    size_t getCountLimit() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _countLimit;
    }
    
    void setCountLimit(size_t countLimit)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_countLimit != countLimit) {
            _countLimit = countLimit;
            enforceLimits();
        }
    }
    
private:
    mutable std::mutex _mutex;
    
    std::unordered_map<size_t, ElementType> _hashMap;
    GridLRU<size_t> _lru;
    size_t _countLimit;
    
    ElementType get(size_t key)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _lru.reference(key);
        return _hashMap[key];
    }
    
    void set(size_t key, const ElementType &el)
    {
        std::lock_guard<std::mutex> lock1(_mutex);
        _lru.reference(key);
        _hashMap[key] = el;
        enforceLimits();
    }
    
    // Must hold the lock on entry to this method.
    void enforceLimits()
    {
        while (_hashMap.size() >= _countLimit) {
            auto maybe = _lru.pop();
            if (maybe) {
                const size_t keyToRemove = *maybe;
                _hashMap.erase(keyToRemove);
            }
        }
    }
};

#endif /* SparseGrid_hpp */
