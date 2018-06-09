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
// associated with an element. It supports multiple, concurrent
// readers and writers.
template<typename Value, size_t NumberOfBuckets = 4093>
class SparseGrid : public GridIndexer
{
public:
    using Key = Morton3;
    
    ~SparseGrid() = default;
    SparseGrid() = delete;
    
    SparseGrid(const AABB &boundingBox,
               const glm::ivec3 &gridResolution,
               size_t numberOfBuckets = NumberOfBuckets)
     : GridIndexer(boundingBox, gridResolution)
    {
        buckets.resize(numberOfBuckets);
    }
    
    // Return the element at the specified index, if there is one.
    boost::optional<Value> get(Key key)
    {
        Bucket& bucket = getBucket(key);
        std::lock_guard<std::mutex> lock(bucket.mutex);
        return bucket.slots[key];
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
        Bucket& bucket = getBucket(key);
        std::lock_guard<std::mutex> lock(bucket.mutex);
        bucket.slots[key] = value;
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
    
    // Removes the element associated with the given index.
    // The element is discarded and the associated slot becomes empty.
    inline void remove(Key key)
    {
        Bucket& bucket = getBucket(key);
        std::lock_guard<std::mutex> lock(bucket.mutex);
        bucket.slots.erase(key);
    }
    
private:
    struct Bucket
    {
        std::mutex mutex;
        std::unordered_map<Key, boost::optional<Value>> slots;
        
        Bucket() = default;
        Bucket(const Bucket &bucket) : slots(bucket.slots) {}
    };
    
    std::vector<Bucket> buckets;
    std::hash<Key> _hasher;
    
    inline Bucket& getBucket(Key key)
    {
        return buckets[_hasher(key) % buckets.size()];
    }
};

#endif /* SparseGrid_hpp */
