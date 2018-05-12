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
    
    boost::optional<ElementType> getIfExists(Morton3 morton)
    {
        std::lock_guard<std::mutex> tableLock(_mutex);
        auto iter = _slots.find(morton);
        if (iter == _slots.end()) {
            return boost::none;
        } else {
            _lru.reference(morton);
            
            std::mutex &slotMutex = _slotMutexes[morton];
            boost::optional<ElementType> &slot = _slots[morton];
            
            // Lock the slot before attempting to retrieve the value.
            std::lock_guard<std::mutex> slotLock(slotMutex);
            return slot;
        }
    }
    
    boost::optional<ElementType> getIfExists(const glm::vec3 &p)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
        }
        return getIfExists(indexAtPoint(p));
    }
    
    template<typename FactoryType>
    ElementType get(Morton3 morton, FactoryType &&factory)
    {
        std::unique_lock<std::mutex> tableLock(_mutex);
        
        _lru.reference(morton); // reference the key so it doesn't get evicted
        enforceLimits();
        
        std::unique_lock<std::mutex> slotLock(_slotMutexes[morton]);
        boost::optional<ElementType> &slot = _slots[morton];
        
        _lru.reference(morton); // reference the key because it's recently used
        tableLock.unlock();
        
        if (!slot) {
            slot = boost::make_optional(factory());
        }
        ElementType el = *slot;
        return el;
    }
    
    ElementType get(Morton3 morton)
    {
        return get(morton, []{ return ElementType(); });
    }
    
    void set(Morton3 morton, const ElementType &el)
    {
        std::unique_lock<std::mutex> tableLock(_mutex);
        
        _lru.reference(morton); // reference the key so it doesn't get evicted
        enforceLimits();
        
        std::unique_lock<std::mutex> slotLock(_slotMutexes[morton]);
        boost::optional<ElementType> &slot = _slots[morton];
        
        _lru.reference(morton); // reference the key because it's recently used
        tableLock.unlock();
        
        slot = boost::make_optional(el);
    }
    
    ElementType get(const glm::vec3 &p)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException();
            }
        }
        return get(indexAtPoint(p));
    }
    
    void set(const glm::vec3 &p, const ElementType &el)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException();
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
    
private:
    // Protects `_slots' and `_slotMutexes' from concurrent access.
    mutable std::mutex _mutex;
    
    // Slots in the sparse grid. Each may contain an element.
    std::unordered_map<Morton3, boost::optional<ElementType>> _slots;
    
    // One lock per cell in the sparse grid.
    std::unordered_map<Morton3, std::mutex> _slotMutexes;
    
    // Track of least-recently used items in case we need to evict some.
    GridLRU<Morton3> _lru;
    
    // Limit on the number elements in the sparse grid. We evict items in LRU
    // order to keep the count at, or under, this limit.
    size_t _countLimit;
    
    // Must hold `_mutex' on entry to this method.
    void enforceLimits()
    {
        assert(_countLimit > 1);
        
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
            
            // Take the lock on the slot before removing it.
            std::mutex &slotMutex = _slotMutexes[key];
            std::lock_guard<std::mutex> lock(slotMutex);
            
            // We never erase locks in _slotMutexes. This is a kind of
            // space leak, but it's not too bad and we can live with it.
            // TODO: Find a good way to purge mutexes for slots which are empty
            // without having to block on all those locks.
            _slots.erase(key);
        }
    }
};

#endif /* SparseGrid_hpp */
