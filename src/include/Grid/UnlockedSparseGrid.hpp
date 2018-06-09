//
//  UnlockedSparseGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/27/17.
//
//

#ifndef UnlockedSparseGrid_hpp
#define UnlockedSparseGrid_hpp

#include "Grid/GridIndexer.hpp"
#include <unordered_map>

// UnlockedSparseGrid divides space into a regular grid of cells where each cell is
// associated with an element.
//
// Access is totally unsynchronized and is not thread-safe.
//
// The grid may limit it's size and choose to evict items which have not been
// used recently.
template<typename Value>
class UnlockedSparseGrid : public GridIndexer
{
public:
    using Key = Morton3;
    
    ~UnlockedSparseGrid() = default;
    UnlockedSparseGrid() = delete;
    
    UnlockedSparseGrid(const AABB &boundingBox,
                       const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution)
    {}
    
    // Return the element at the specified index, if there is one.
    boost::optional<Value> get(Key key)
    {
        auto iter = _dictionary.find(key);
        if (iter == _dictionary.end()) {
            return boost::none;
        } else {
            return iter->second;
        }
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
        _dictionary[key] = value;
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
        _dictionary.erase(key);
    }
    
private:
    std::unordered_map<Key, Value> _dictionary;
};

#endif /* UnlockedSparseGrid_hpp */
