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
    
    // Copy assignment operator. This is done unlocked so be careful.
    SparseGrid<ElementType>& operator=(const SparseGrid<ElementType> &other)
    {
        if (&other != this) {
            _slots.clear();
            
            for (auto &outerPair : other._slots) {
                Morton3 key = outerPair.first;
                _slots[key].second = outerPair.second.second;
            }
            
            _lru = other._lru;
            _countLimit = other._countLimit;
        }
        return *this;
    }
    
    boost::optional<ElementType> getIfExists(const Morton3 &morton)
    {
        std::lock_guard<std::mutex> lock1(_mutex);
        auto iter = _slots.find(morton);
        if (iter == _slots.end()) {
            return boost::none;
        } else {
            _lru.reference(morton);
            
            // Lock the slot before attempting to retrieve the value.
            std::pair<std::mutex, Slot> &pair = iter->second;
            std::lock_guard<std::mutex> lock2(pair.first);
            Slot &slot = pair.second;
            return slot;
        }
    }
    
    boost::optional<ElementType> getIfExists(const glm::vec3 &p)
    {
#ifdef EnableVerboseBoundsChecking
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
#endif
        return getIfExists(indexAtPoint(p));
    }
    
    template<typename FactoryType>
    ElementType get(const Morton3 &morton, FactoryType &&factory)
    {
        std::pair<std::mutex, Slot> *ppair = nullptr;
        
        // Get a reference to the slot under lock.
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _lru.reference(morton);
            ppair = &_slots[morton];
            enforceLimits();
        }
        
        // Get the value in the slot using the slot-specific lock.
        {
            std::mutex &slotMutex = ppair->first;
            Slot &slot = ppair->second;
            
            std::lock_guard<std::mutex> lock(slotMutex);
            if (!slot) {
                slot = boost::make_optional(factory());
            }
            return *slot;
        }
    }
    
    ElementType get(const Morton3 &morton)
    {
        return get(morton, []{ return ElementType(); });
    }
    
    void set(const Morton3 &morton, const ElementType &el)
    {
        std::pair<std::mutex, Slot> *ppair = nullptr;
        
        // Get a reference to the slot under lock.
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _lru.reference(morton);
            ppair = &_slots[morton];
            enforceLimits();
        }
        
        // Set the value in the slot using the slot-specific lock.
        {
            std::mutex &slotMutex = ppair->first;
            Slot &slot = ppair->second;
            
            std::lock_guard<std::mutex> lock(slotMutex);
            slot = boost::make_optional(el);
        }
    }
    
    ElementType get(const glm::vec3 &p)
    {
#ifdef EnableVerboseBoundsChecking
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
#endif
        return get(indexAtPoint(p));
    }
    
    void set(const glm::vec3 &p, const ElementType &el)
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
    using Slot = boost::optional<ElementType>;
    
    mutable std::mutex _mutex;
    std::unordered_map<Morton3, std::pair<std::mutex, Slot>> _slots;
    GridLRU<Morton3> _lru;
    size_t _countLimit;
    
    // Must hold `_mutex' on entry to this method.
    void enforceLimits()
    {
        while (_slots.size() >= _countLimit) {
            auto maybeKey = _lru.pop();
            if (maybeKey) {
                const Morton3 key = *maybeKey;
                auto slotIter = _slots.find(key);

                if (slotIter != _slots.end()) {
                    std::pair<std::mutex, Slot> &pair = slotIter->second;
                    std::mutex &slotMutex = pair.first;
                    
                    // Take the lock on the slot before removing it.
                    std::lock_guard<std::mutex> lock(slotMutex);
                    _slots.erase(slotIter);
                }
            }
        }
    }
};

#endif /* SparseGrid_hpp */
