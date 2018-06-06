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
#include <mutex>

// SparseGrid divides space into a regular grid of cells where each cell is
// associated with an element.
//
// The grid may limit it's size and choose to evict items which have not been
// used recently.
//
// Access to the grid is synchronized so that concurrent access is safe.
// However, clients may see data races associated with multiple, concurrent
// accesses to the same element. Enforce synchronization at a higher level to
// ensure consistency.
template<typename ElementType>
class SparseGrid : public GridIndexer
{
public:
    ~SparseGrid() = default;
    SparseGrid() = delete;
    
    SparseGrid(const AABB &boundingBox, const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution),
       _countLimit(std::numeric_limits<std::size_t>::max()),
       _suspendLimitEnforcement(0)
    {}
    
    // Copy assignment operator. This is done unlocked so be careful.
    SparseGrid<ElementType>& operator=(const SparseGrid<ElementType> &other)
    {
        if (&other != this) {
            _slots.clear();
            
            for (auto &pair : other._slots) {
                Morton3 key = pair.first;
                _slots[key] = pair.second;
            }
            
            _lru = other._lru;
            _countLimit = other._countLimit;
        }
        return *this;
    }
    
    // Return the element at the specified index, if there is one.
    boost::optional<ElementType> get(Morton3 morton)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto iter = _slots.find(morton);
        if (iter == _slots.end()) {
            return boost::none;
        } else {
            _lru.reference(morton);
            boost::optional<ElementType> &slot = _slots[morton];
            return slot;
        }
    }
    
    // Return the element at the specified index.
    // If the slot is empty then this uses `factory' to populate the slot.
    template<typename FactoryType>
    ElementType get(Morton3 morton, FactoryType &&factory)
    {
        auto maybeChunk = get(morton);
        if (!maybeChunk) {
            set(morton, factory());
            maybeChunk = get(morton);
        }
        assert(maybeChunk);
        return *maybeChunk;
    }
    
    // Set the element at the specified index to the specified value.
    void set(Morton3 morton, const ElementType &el)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _lru.reference(morton); // reference the key so it doesn't get evicted
        enforceLimits();
        boost::optional<ElementType> &slot = _slots[morton];
        _lru.reference(morton); // reference the key because it's recently used
        slot = boost::make_optional(el);
    }
    
    // Set the element at the specified point to the specified value.
    void set(const glm::vec3 &p, const ElementType &el)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException -- boundingBox={} ; p={}",
                                                       boundingBox(),
                                                       glm::to_string(p)));
            }
        }
        return set(indexAtPoint(p), el);
    }
    
    // Returns the maximum number elements in the sparse grid
    // before we start to evict items.
    size_t getCountLimit() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _countLimit;
    }
    
    // Limit on the number elements in the sparse grid. We evict items in LRU
    // order to keep the count at, or under, this limit.
    void setCountLimit(size_t countLimit)
    {
        assert(countLimit >= 1);
        std::lock_guard<std::mutex> lock(_mutex);
        if (_countLimit != countLimit) {
            _countLimit = countLimit;
            enforceLimits();
        }
    }
    
    // Sometimes, for correctness, we need to suspend eviction of chunks until
    // an operation is finished. For example, we may need to modify several
    // chunks and then save them back to file. Evicting one before we can save
    // the changes will basically throw away the work and lead to incorrect
    // results.
    void suspendLimitEnforcement()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _suspendLimitEnforcement++;
    }
    
    // Resume limit enforcement after it had been suspended for a while.
    void resumeLimitEnforcement()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _suspendLimitEnforcement--;
        if (_suspendLimitEnforcement > 0) {
            enforceLimits();
        }
    }
    
    // Removes the element associated with the given index.
    // The element is discarded and the associated slot becomes empty.
    void remove(Morton3 morton)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        boost::optional<ElementType> &slot = _slots[morton];
        slot = boost::none;
    }
    
private:
    mutable std::mutex _mutex;
    
    // Slots in the sparse grid. Each may contain an element.
    std::unordered_map<Morton3, boost::optional<ElementType>> _slots;
    
    // Track of least-recently used items in case we need to evict some.
    GridLRU<Morton3> _lru;
    
    // Limit on the number elements in the sparse grid. We evict items in LRU
    // order to keep the count at, or under, this limit.
    size_t _countLimit;
    
    // Enforcement of grid limits is suspended so long as this counter is > 0.
    int _suspendLimitEnforcement;
    
    // Must hold `_mutex' on entry to this method.
    void enforceLimits()
    {
        assert(_countLimit > 1);
        
        if (_suspendLimitEnforcement > 0) {
            return;
        }
        
        while (_slots.size() > _countLimit) {
            auto maybeKey = _lru.pop();
            Morton3 key;
            assert(maybeKey);
            
            if (maybeKey) {
                key = *maybeKey;
            } else {
                // There are no more items in the LRU list. Since we have to
                // evict something, let's pick the first item in the table.
                key = _slots.begin()->first;
            }
            
            _slots.erase(key);
        }
    }
};

#endif /* SparseGrid_hpp */
