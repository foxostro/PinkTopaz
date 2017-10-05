//
//  MallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#ifndef MallocZone_hpp
#define MallocZone_hpp

#include "Malloc/MallocBlock.hpp"

class MallocZone {
    MallocBlock *_head;

public:
    // Initializes the malloc zone using the specified region of memory.
    // Returns the address of the MallocZone structure stored within the memory.
    // Allocations from the zone will always be taken from this memory region.
    MallocZone(uint8_t *start, size_t size);

    // Allocates a block of memory of the given size from the malloc zone.
    // May return nullptr if the request cannot be satisfied.
    // If size is zero a new minimum-sized object is allocated.
    uint8_t* malloc_zone_malloc(size_t size);

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
    uint8_t* malloc_zone_realloc(uint8_t *ptr, size_t newSize);

    // Deallocates a memory allocation pointed to be ptr. If ptr is nullptr then
    // no operation is performed.
    void malloc_zone_free(uint8_t *ptr);

    inline MallocBlock* head() const {
        return _head;
    }
};

#endif /* MallocZone_hpp */
