//
//  GridLRU.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/26/17.
//

#ifndef GridLRU_hpp
#define GridLRU_hpp

#include <boost/optional.hpp>
#include <map>
#include <list>

// Tracks Least-Recently Used items, where each item may be identified by a key.
template<typename KeyType>
class GridLRU
{
public:
    // Mark the item as being recently used.
    void reference(KeyType key)
    {
        if (auto mapIter = _lookup.find(key); mapIter != _lookup.end()) {
            // Move the element to the end of the list.
            auto listIter = mapIter->second;
            _list.splice(_list.end(), _list, listIter);
        } else {
            // Insert a new element at the end of the list.
            _list.push_back(key);
            auto backIter = _list.end();
            backIter--;
            _lookup[key] = backIter;
        }
    }
    
    // Get the least recently used item and remove it from the LRU list.
    // Returns an empty optional when there is no item to return.
    boost::optional<KeyType> pop()
    {
        if (_list.empty()) {
            return boost::none;
        }
        
        // Remove the tail item from the list.
        auto backIter = _list.end();
        backIter--;
        const KeyType key = *backIter;
        _list.erase(backIter);
        _lookup.erase(key);
        
        return boost::make_optional(key);
    }
    
    // Remove all items from the LRU list.
    void clear()
    {
        _list.clear();
        _lookup.clear();
    }
    
    // Copy assignment operator.
    GridLRU<KeyType>& operator=(const GridLRU<KeyType> &other)
    {
        if (&other != this) {
            _list = other._list;
            
            // Can't copy the iterators, so rebuild the lookup table.
            for (auto iter = _list.begin(); iter != _list.end(); ++iter) {
                _lookup[*iter] = iter;
            }
        }
        return *this;
    }
    
private:
    std::list<KeyType> _list;
    
    using list_iterator = typename std::list<KeyType>::iterator;
    std::map<KeyType, list_iterator> _lookup;
};

#endif /* GridLRU_hpp */
