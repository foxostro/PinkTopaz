//
//  MallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Malloc/MallocZone.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>

constexpr size_t ALIGN = 4;
constexpr size_t MIN_SPLIT_SIZE = sizeof(MallocZone::Block);

static inline size_t roundUpBlockSize(size_t size)
{
    size_t newSize = ((size + ALIGN - 1) / ALIGN) * ALIGN;

    if (newSize < size) { // check for overflow
        newSize = SIZE_MAX - 3;
        assert(newSize % ALIGN == 0);
    }

    return newSize;
}

void MallocZone::considerSplittingBlock(Block *block, size_t size)
{
    // Split the block if the remaining free space is big enough.
    size_t remainingSpace = block->size - size;
    if (remainingSpace > MIN_SPLIT_SIZE) {
        Block *newBlock = (Block *)((uint8_t *)block + sizeof(Block) + size);

        assert((uintptr_t)newBlock % ALIGN == 0);
        newBlock->setPrev(*this, block);
        newBlock->setNext(*this, block->getNext(*this));
        newBlock->size = remainingSpace - sizeof(Block);
        newBlock->inuse = false;

        block->size = size;
        if (block->getNext(*this)) {
            block->getNext(*this)->setPrev(*this, newBlock);
        }
        block->setNext(*this, newBlock);

        // If the next block is empty then merge the new free block with the
        // free block that follows it.
        Block *following = newBlock->getNext(*this);
        if (following && !following->inuse) {
            newBlock->size += following->size + sizeof(Block);
            newBlock->setNext(*this, following->getNext(*this));
            if (following->getNext(*this)) {
                following->getNext(*this)->setPrev(*this, newBlock);
            }
        }
    }
}

MallocZone::MallocZone(uint8_t *start, size_t size)
{
    assert(start);
    assert(size > sizeof(MallocZone::Block));
    memset(start, 0, size);
    
    _start = start;
    _size = size;
    _head = (Block *)(start + (4 - (uintptr_t)start % 4)); // 4 byte alignment
    
    _head->setPrev(*this, nullptr);
    _head->setNext(*this, nullptr);
    _head->size = size - sizeof(Block);
    _head->inuse = false;
}

uint8_t* MallocZone::allocate(size_t size)
{
    // Blocks for allocations are always multiples of four bytes in size.
    // This ensures that blocks are always aligned on four byte boundaries
    // given that the initial block is also aligned on a four byte boundary.
    size = roundUpBlockSize(size);

    Block *best = nullptr;

    // Get the smallest free block that is large enough to satisfy the request.
    for (Block *block = _head; block; block = block->getNext(*this)) {
        if (block->size >= size
            && !block->inuse
            && (!best || (block->size < best->size))) {
            best = block;
        }
    }

    if (!best) {
        return nullptr;
    }

    considerSplittingBlock(best, size);

    best->inuse = true;
    return (uint8_t *)best + sizeof(Block);
}

void MallocZone::deallocate(uint8_t *ptr)
{
    if (!ptr) {
        return; // do nothing
    }

    Block *block = (Block *)(ptr - sizeof(Block));

    // Walk over the heap and see if we can find this allocation.
    // If we cannot find it then the calling code has an error in it.
#ifndef NDEBUG
    bool foundIt = false;
    for (Block *iter = _head; iter; iter = iter->getNext(*this)) {
        if (iter == block) {
            foundIt = true;
        }
    }
    assert(foundIt);
    assert(block->inuse);
#endif

    block->inuse = false;

    Block *preceding = block->getPrev(*this), *following = block->getNext(*this);

    // If the preceding chunk is free then merge this one into it. This block
    // goes away and the preceding chunk expands to fill the hole.
    if (preceding && !preceding->inuse) {
        preceding->size += block->size + sizeof(Block);
        preceding->setNext(*this, following);
        if (following) {
            following->setPrev(*this, preceding);
        }

        // Reset these pointers for a possible merge with `following', below.
        block = preceding;
        preceding = block->getPrev(*this);
    }

    // If the following chunk is free then merge it into this one.
    // The following block goes away and this chunk expands to fill the hole.
    if (following && !following->inuse) {
        block->size += following->size + sizeof(Block);
        block->setNext(*this, following->getNext(*this));
        if (following->getNext(*this)) {
            following->getNext(*this)->setPrev(*this, block);
        }
    }
}

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
uint8_t* MallocZone::reallocate(uint8_t *ptr, size_t newSize)
{
    if (!ptr) {
        return allocate(newSize);
    }

    Block *block = (Block *)(ptr - sizeof(Block));
    assert(block->inuse);

    // Walk over the heap and see if we can find this allocation.
    // If we cannot find it then the calling code has an error in it.
#ifndef NDEBUG
    bool foundIt = false;
    for (Block *iter = _head; iter; iter = iter->getNext(*this)) {
        if (iter == block) {
            foundIt = true;
        }
    }
    assert(foundIt);
#endif

    if (newSize == 0) {
        deallocate(ptr);
        return allocate(0);
    }

    // Blocks for allocations are always multiples of four bytes in size.
    // This ensures that blocks are always aligned on four byte boundaries
    // given that the initial block is also aligned on a four byte boundary.
    newSize = roundUpBlockSize(newSize);

    // The block is already large enough to accomodate the new size.
    if (block->size >= newSize) {
        considerSplittingBlock(block, newSize);
        return ptr;
    }

    Block *following = block->getNext(*this);

    // If this block is followed by a free block then would it satisfy our
    // requirements to take some of that free space by extending this block?
    if (following
        && !following->inuse
        && (block->size + following->size + sizeof(Block)) >= newSize) {

        // Remove the following block, extending this one so as to not leave a
        // hole in the zone.
        block->size = block->size + following->size + sizeof(Block);
        block->setNext(*this, following->getNext(*this));

        if (following->getNext(*this)) {
            following->getNext(*this)->setPrev(*this, block);
        }

        // Split the remaining free space if there's enough of it.
        considerSplittingBlock(block, newSize);

        return ptr;
    }

    // Can we allocate a new block of memory for the resized allocation?
    uint8_t *newAlloc = allocate(newSize);
    if (newAlloc) {
        memcpy(ptr, newAlloc, block->size);
        deallocate(ptr);
        return newAlloc;
    }

    Block *preceding = block->getPrev(*this);

    // If this block is preceded by a free block then would it satisfy our
    // requirements to merge into the preceding block?
    if (preceding
        && !preceding->inuse
        && (block->size + preceding->size + sizeof(Block)) >= newSize) {

        // Remove this block, extending the preceding one so as to not leave a
        // hole in the zone.
        preceding->setNext(*this, following);
        if (following) {
            following->setPrev(*this, preceding);
        }
        preceding->size = block->size + preceding->size + sizeof(Block);
        preceding->inuse = true;

        // Move the contents to the beginning of the new, combined block.
        newAlloc = (uint8_t *)preceding + sizeof(Block);
        memmove(ptr, newAlloc, block->size);

        // Split the remaining free space if there's enough of it.
        considerSplittingBlock(preceding, newSize);

        return newAlloc;
    }

    return nullptr;
}
