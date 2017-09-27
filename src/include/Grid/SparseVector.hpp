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

// Intended to be a drop-in replacement for std::vector for use in BaseArray3D.
// Elements of this container are not allocated storage until they are used.
template<typename ElementType>
class SparseVector
{
public:
    SparseVector() {}
    SparseVector(size_t capacity) {}
    
    // Copy constructor.
    SparseVector(const SparseVector<ElementType> &a)
     : _data(a._data)
    {}
    
    // Move constructor.
    SparseVector(SparseVector<ElementType> &&a)
     : _data(std::move(a._data))
    {}
    
    // Copy assignment operator.
    // We need this because we have a user-declared move constructor.
    SparseVector<ElementType>& operator=(const SparseVector<ElementType> &a)
    {
        _data = a._data;
        return *this;
    }
    
    inline ElementType& operator[](size_t i)
    {
        std::lock_guard<std::mutex> lock(_lock);
        return _data[i];
    }
    
    inline const ElementType& operator[](size_t i) const
    {
        std::lock_guard<std::mutex> lock(_lock);
        return _data[i];
    }
    
    inline void clear()
    {
        std::lock_guard<std::mutex> lock(_lock);
        _data.clear();
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
};

#endif /* SparseVector_hpp */
