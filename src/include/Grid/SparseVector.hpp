//
//  SparseVector.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/26/17.
//
//

#ifndef SparseVector_hpp
#define SparseVector_hpp

#include <unordered_map>
#include <mutex>
#include "GridLRU.hpp"

// Elements of this container are not allocated storage until they are used.
template<typename ElementType>
class SparseVector
{
public:
    SparseVector() : _countLimit(std::numeric_limits<std::size_t>::max()) {}
    SparseVector(size_t capacity) : SparseVector() {}
    
    // Copy constructor.
    SparseVector(const SparseVector<ElementType> &a)
     : _data(a._data),
       _countLimit(a._countLimit),
       _lru(a._lru)
    {}
    
    // Move constructor.
    SparseVector(SparseVector<ElementType> &&a)
     : _data(std::move(a._data)),
       _countLimit(a._countLimit),
       _lru(std::move(a._lru))
    {}
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    SparseVector<ElementType>& operator=(const SparseVector<ElementType> &a)
    {
        _data = a._data;
        _lru = a._lru;
        _countLimit = a._countLimit;
        return *this;
    }
    
    inline ElementType& operator[](size_t i)
    {
        std::lock_guard<std::mutex> lock(_lock);
        _lru.reference(i);
        return _data[i];
    }
    
    inline const ElementType& operator[](size_t i) const
    {
        std::lock_guard<std::mutex> lock(_lock);
        _lru.reference(i);
        return _data[i];
    }
    
    inline void clear()
    {
        std::lock_guard<std::mutex> lock(_lock);
        _data.clear();
        _lru.clear();
    }
    
    inline void setCountLimit(size_t countLimit)
    {
        std::lock_guard<std::mutex> lock(_lock);
        _countLimit = countLimit;
    }
    
    inline size_t getCountLimit() const
    {
        std::lock_guard<std::mutex> lock(_lock);
        return _countLimit;
    }
    
    // Remove elements until the number of items is under the limit.
    // Limits are only enforced on a call to enforceLimits().
    // It is not safe to call this method while other threads are using
    // references obtained through operator[]. It is the user's responsibility
    // to ensure this does not occur.
    void enforceLimits()
    {
        std::lock_guard<std::mutex> lock(_lock);
        while (_data.size() > _countLimit) {
            boost::optional<size_t> item = _lru.pop();
            if (item) {
                size_t index = *item;
                _data.erase(index);
            }
        }
    }
    
private:
    // We must add a lock and make sure to protect our internal data structures
    // from concurrent accesses. We do this because users expect to be able to
    // access this container from many concurrent readers without any locking
    // on their end. This is appropriate for std::vector, but not for
    // std::unordered_map.
    mutable std::mutex _lock;
    
    // `_data' must be marked as "mutable" because we need to be able to get
    // a const-reference to an element in the const operator[]. However,
    // unordered_map will modify the buckets list to insert an element if one is
    // not present for the specified index.
    mutable std::unordered_map<size_t, ElementType> _data;
    
    // `_lru' must be mutable so we can note element access from inside the
    // const operator[].
    mutable GridLRU<size_t> _lru;
    
    // We limit the size of the sparse vector to `_countLimit' items. If the
    // number of items exceeds this limit then we remove the least-recently
    // used element until we're under the limit.
    size_t _countLimit;
};

#endif /* SparseVector_hpp */
