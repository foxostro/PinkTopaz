//
//  DenseVector.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/26/17.
//
//

#ifndef DenseVector_hpp
#define DenseVector_hpp

#include <vector>

template<typename ElementType>
class DenseVector : public std::vector<ElementType>
{
public:
    DenseVector() = default;
    DenseVector(size_t capacity) : std::vector<ElementType>(capacity) {}
    
    inline void setCountLimit(size_t countLimit)
    {
        // nothing to do
    }
    
    inline size_t getCountLimit() const
    {
        return std::numeric_limits<std::size_t>::max();
    }
    
    void enforceLimits()
    {
        // nothing to do
    }
};

#endif /* DenseVector_hpp */

