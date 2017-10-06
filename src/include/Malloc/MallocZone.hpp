//
//  MallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#ifndef MallocZone_hpp
#define MallocZone_hpp

#include <cstdint>
#include <cstddef>

class MallocZone
{
public:
    // ~0 indicates that we do not have a valid offset
    static constexpr ptrdiff_t sentinel = ~0;
    
    class Block
    {
    public:
        ptrdiff_t prevOffset;
        ptrdiff_t nextOffset;
        size_t size;
        unsigned inuse;
        
        Block()
        : prevOffset(sentinel),
          nextOffset(sentinel),
          size(0),
          inuse(0)
        {}
        
        inline void setPrev(MallocZone &zone, Block *prev)
        {
            if (zone.validate((uint8_t *)prev)) {
                prevOffset = (uint8_t *)prev - zone.start();
            } else {
                prevOffset = sentinel;
            }
        }
        
        inline void setNext(MallocZone &zone, Block *next)
        {
            if (zone.validate((uint8_t *)next)) {
                nextOffset = (uint8_t *)next - zone.start();
            } else {
                nextOffset = sentinel;
            }
        }
        
        inline Block* getPrev(MallocZone &zone)
        {
            return (prevOffset == sentinel) ? nullptr : (Block *)(zone.start() + prevOffset);
        }
        
        inline Block* getNext(MallocZone &zone)
        {
            return (nextOffset == sentinel) ? nullptr : (Block *)(zone.start() + nextOffset);
        }
    };

public:
    // Initializes the malloc zone using the specified region of memory.
    // Returns the address of the MallocZone structure stored within the memory.
    // Allocations from the zone will always be taken from this memory region.
    MallocZone(uint8_t *start, size_t size);

    // Allocates a block of memory of the given size from the malloc zone.
    // May return nullptr if the request cannot be satisfied.
    // If size is zero a new minimum-sized object is allocated.
    uint8_t* allocate(size_t size);

    // Tries to change the size of the allocation pointed to by ptr to size,
    // and returns the address of the new allocation. This may move the
    // allocation to a different part of the heap, copying as much of the old
    // data pointed to by ptr as will fit in the new allocation, and freeing the
    // old allocation.
    // 
    // When extending an allocation, no guarantee is made as to the value of
    // memory in the extended region.
    // 
    // If ptr is nullptr, realloc() is identical to a call to malloc().
    // 
    // If size is zero and ptr is not nullptr, a new minimum-sized object is
    // allocated and the original object is freed.
    uint8_t* reallocate(uint8_t *ptr, size_t newSize);

    // Deallocates a memory allocation pointed to be ptr. If ptr is nullptr then
    // no operation is performed.
    void deallocate(uint8_t *ptr);

    inline Block* head() const {
        return _head;
    }
    
    inline uint8_t *start() const {
        return _start;
    }
    
    inline bool validate(uint8_t *ptr) const {
        return ptr >= _start && ptr < (_start + _size);
    }
    
private:
    uint8_t *_start;
    size_t _size;
    Block *_head;
    
    void considerSplittingBlock(MallocZone::Block *block, size_t size);
};

#endif /* MallocZone_hpp */
