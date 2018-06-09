//
//  LimitedConcurrentSparseGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/27/17.
//
//

#ifndef LimitedConcurrentSparseGrid_hpp
#define LimitedConcurrentSparseGrid_hpp

#include "Grid/GridIndexer.hpp"
#include "Grid/GridLRU.hpp"
#include <unordered_map>
#include <mutex>

// LimitedConcurrentSparseGrid divides space into a regular grid of cells where
// each cell is associated with an element. It supports multiple, concurrent
// readers and writers.
//
// The grid may limit it's size and choose to evict items which have not been
// used recently.
template<typename Value, size_t NumberOfBuckets = 4093>
class LimitedConcurrentSparseGrid : public GridIndexer
{
public:
    using Key = Morton3;
    
    ~LimitedConcurrentSparseGrid() = default;
    LimitedConcurrentSparseGrid() = delete;
    
    LimitedConcurrentSparseGrid(const AABB &boundingBox,
                                const glm::ivec3 &gridResolution,
                                size_t numberOfBuckets = NumberOfBuckets)
     : GridIndexer(boundingBox, gridResolution),
       _countLimit(std::numeric_limits<std::size_t>::max()),
       _count(0),
       _suspendLimitEnforcement(0)
    {
        _buckets.resize(numberOfBuckets);
    }
    
    // Return the element at the specified index, if there is one.
    boost::optional<Value> get(Key key)
    {
        Bucket& bucket = getBucket(key);
        std::scoped_lock<std::mutex, std::mutex> lock(bucket.mutex, _mutexLRU);
        auto maybeValue = bucket.slots[key];
        if (maybeValue) {
            _lru.reference(key);
        }
        return maybeValue;
    }
    
    // Return the element at the specified index.
    // If the slot is empty then this uses `factory' to populate the slot.
    template<typename FactoryType>
    Value get(Key key, FactoryType &&factory)
    {
        auto maybeChunk = get(key);
        if (!maybeChunk) {
            set(key, factory());
            maybeChunk = get(key);
        }
        assert(maybeChunk);
        return *maybeChunk;
    }
    
    // Set the element at the specified index to the specified value.
    void set(Key key, Value value)
    {
        {
            Bucket& bucket = getBucket(key);
            std::scoped_lock lock(bucket.mutex, _mutexLRU, _mutexCount, _mutexCountLimit);
            auto& slot = bucket.slots[key];
            if (slot != boost::none) {
                _lru.reference(key);
                _count++;
            }
            slot = value;
        }
        
        enforceLimits();
    }
    
    // Set the element at the specified point to the specified value.
    void set(const glm::vec3 &p, Value value)
    {
        if constexpr (EnableVerboseBoundsChecking) {
            if (!inbounds(p)) {
                throw OutOfBoundsException(fmt::format("OutOfBoundsException -- boundingBox={} ; p={}",
                                                       boundingBox(),
                                                       glm::to_string(p)));
            }
        }
        return set(indexAtPoint(p), value);
    }
    
    // Returns the maximum number elements in the sparse grid
    // before we start to evict items.
    size_t getCountLimit() const
    {
        std::scoped_lock lock(_mutexCountLimit);
        return _countLimit;
    }
    
    // Limit on the number elements in the sparse grid. We evict items in LRU
    // order to keep the count at, or under, this limit.
    void setCountLimit(size_t countLimit)
    {
        assert(countLimit >= 1);
        
        {
            std::scoped_lock lock(_mutexCountLimit);
            if (_countLimit != countLimit) {
                _countLimit = countLimit;
                
            }
        }
        
        enforceLimits();
    }
    
    // Sometimes, for correctness, we need to suspend eviction of chunks until
    // an operation is finished. For example, we may need to modify several
    // chunks and then save them back to file. Evicting one before we can save
    // the changes will basically throw away the work and lead to incorrect
    // results.
    void suspendLimitEnforcement()
    {
        std::scoped_lock<std::mutex> lock(_mutexSuspendLimitEnforcement);
        _suspendLimitEnforcement++;
    }
    
    // Resume limit enforcement after it had been suspended for a while.
    void resumeLimitEnforcement()
    {
        bool mustEnforceLimits;
        
        {
            std::scoped_lock lock(_mutexSuspendLimitEnforcement);
            _suspendLimitEnforcement--;
            mustEnforceLimits = (_suspendLimitEnforcement > 0);
        }
        
        if (mustEnforceLimits) {
            enforceLimits();
        }
    }
    
    // Removes the element associated with the given index.
    // The element is discarded and the associated slot becomes empty.
    void remove(Key key)
    {
        Bucket& bucket = getBucket(key);
        std::scoped_lock lock(bucket.mutex, _mutexCount);
        size_t numberOfElementsErased = bucket.slots.erase(key);
        assert(_count >= numberOfElementsErased);
        _count -= numberOfElementsErased;
    }
    
    // Must not be holding any of the mutexes on entry to this method.
    void enforceLimits()
    {
        std::scoped_lock lock(_mutexLRU, _mutexCount, _mutexCountLimit, _mutexSuspendLimitEnforcement);
        
        if (_suspendLimitEnforcement > 0) {
            return;
        }
        
        while (_count > _countLimit) {
            auto maybeKey = _lru.pop();
            assert(maybeKey);
            Key key = *maybeKey;
            Bucket& bucket = getBucket(key);
            std::lock_guard<std::mutex> lock(bucket.mutex);
            size_t numberOfElementsErased = bucket.slots.erase(key);
            assert(_count >= numberOfElementsErased);
            _count -= numberOfElementsErased;
        }
    }
    
private:
    struct Bucket
    {
        std::mutex mutex;
        std::unordered_map<Key, boost::optional<Value>> slots;
        
        Bucket() = default;
        Bucket(const Bucket &bucket) : slots(bucket.slots) {}
    };
    
    // Track of least-recently used items in case we need to evict some.
    std::mutex _mutexLRU;
    GridLRU<Morton3> _lru;
    
    // Limit on the number elements in the sparse grid. We evict items in LRU
    // order to keep the count at, or under, this limit.
    std::mutex _mutexCountLimit;
    size_t _countLimit;
    
    // Counts the number of values in the grid.
    std::mutex _mutexCount;
    size_t _count;
    
    // Enforcement of grid limits is suspended so long as this counter is > 0.
    std::mutex _mutexSuspendLimitEnforcement;
    int _suspendLimitEnforcement;
    
    std::vector<Bucket> _buckets;
    std::hash<Key> _hasher;
    
    inline Bucket& getBucket(Key key)
    {
        return _buckets[_hasher(key) % _buckets.size()];
    }
};

#endif /* LimitedConcurrentSparseGrid_hpp */
