//
//  GridIndexerRange.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/18/17.
//
//

#ifndef GridIndexerRange_hpp
#define GridIndexerRange_hpp

#include "GridIndexer.hpp"

template<typename PointType>
class ExclusiveIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = PointType;
    using difference_type = PointType;
    using pointer = PointType*;
    using reference = PointType&;
    
    ExclusiveIterator() = delete;
    
    ExclusiveIterator(PointType min, PointType max, PointType pos)
     : _min(min), _max(max), _pos(pos)
    {}
    
    ExclusiveIterator(const ExclusiveIterator<PointType> &a) = default;
    
    ExclusiveIterator<PointType>& operator++()
    {
        ++_pos.y;
        if (_pos.y >= _max.y) {
            _pos.y = _min.y;
            
            ++_pos.x;
            if (_pos.x >= _max.x) {
                _pos.x = _min.x;
                
                ++_pos.z;
                if (_pos.z >= _max.z) {
                    // sentinel value for the end of the sequence
                    _pos = _max;
                }
            }
        }
        
        return *this;
    }
    
    ExclusiveIterator<PointType> operator++(int)
    {
        ExclusiveIterator<PointType> temp(*this);
        operator++();
        return temp;
    }
    
    PointType operator*()
    {
        return _pos;
    }
    
    PointType* operator->()
    {
        return &_pos;
    }
    
    bool operator==(const ExclusiveIterator<PointType> &a) const
    {
        return _min == a._min &&
        _max == a._max &&
        _pos == a._pos;
    }
    
    bool operator!=(const ExclusiveIterator<PointType> &a) const
    {
        return !(*this == a);
    }
    
private:
    PointType _min, _max, _pos;
};

template<typename PointType>
class InclusiveIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = PointType;
    using difference_type = PointType;
    using pointer = PointType*;
    using reference = PointType&;
    
    InclusiveIterator() = delete;
    
    InclusiveIterator(PointType min, PointType max,
                      PointType pos, PointType step,
                      bool done)
    : _min(min), _max(max), _pos(pos), _step(step), _done(done)
    {}
    
    InclusiveIterator(const InclusiveIterator<PointType> &a) = default;
    
    InclusiveIterator<PointType>& operator++()
    {
        if (!_done) {
            _pos.y += _step.y;
            if (_pos.y > _max.y) {
                _pos.y = _min.y;
                
                _pos.x += _step.x;
                if (_pos.x > _max.x) {
                    _pos.x = _min.x;
                    
                    _pos.z += _step.z;
                    if (_pos.z > _max.z) {
                        _pos.z = _min.z;
                        
                        // sentinel value for the end of the sequence
                        _done = true;
                    }
                }
            }
        }
        
        return *this;
    }
    
    InclusiveIterator<PointType> operator++(int)
    {
        InclusiveIterator<PointType> temp(*this);
        operator++();
        return temp;
    }
    
    PointType operator*()
    {
        return _pos;
    }
    
    PointType* operator->()
    {
        return &_pos;
    }
    
    bool operator==(const InclusiveIterator<PointType> &a) const
    {
        return _min == a._min &&
        _max == a._max &&
        _pos == a._pos &&
        _done == a._done;
    }
    
    bool operator!=(const InclusiveIterator<PointType> &a) const
    {
        return !(*this == a);
    }
    
private:
    PointType _min, _max, _pos, _step;
    bool _done;
};

template<typename IteratorType>
class Range
{
public:
    Range() = delete;
    Range(IteratorType a, IteratorType b) : _begin(a), _end(b) {}
    
    IteratorType begin() const
    {
        return _begin;
    }
    
    IteratorType end() const
    {
        return _end;
    }
    
private:
    IteratorType _begin, _end;
};

// Return a Range object which can iterate over a sub-region of the grid.
static inline auto slice(const GridIndexer &grid, const AABB &region)
{
#ifdef EnableVerboseBoundsChecking
    if (!grid.inbounds(region)) {
        const std::string bboxStr = grid.boundingBox().to_string();
        const std::string regionStr = region.to_string();
        throw OutOfBoundsException("grid.boundingBox=%s ; region=%s",
                                   bboxStr.c_str(), regionStr.c_str());
    }
#endif
    
    const auto minCellCoords = grid.cellCoordsAtPoint(region.mins());
    const auto maxCellCoords = grid.cellCoordsAtPointRoundUp(region.maxs());
    
    ExclusiveIterator<glm::ivec3> begin(minCellCoords, maxCellCoords, minCellCoords);
    ExclusiveIterator<glm::ivec3> end(minCellCoords, maxCellCoords, maxCellCoords);
    
    return Range<ExclusiveIterator<glm::ivec3>>(begin, end);
}

#endif /* GridIndexerRange_hpp */
