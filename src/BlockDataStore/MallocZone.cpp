//
//  MallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "BlockDataStore/MallocZone.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>

constexpr bool VERBOSE = false;

constexpr size_t ALIGN = alignof(MallocZone::Block);
constexpr size_t MIN_SPLIT_SIZE = sizeof(MallocZone::Block);

inline size_t roundUpBlockSize(size_t size)
{
    size_t newSize = ((size + ALIGN - 1) / ALIGN) * ALIGN;

    if (newSize < size) { // check for overflow
        newSize = SIZE_MAX - 3;
        assert(newSize % ALIGN == 0);
    }

    return newSize;
}

MallocZone::MallocZone()
 : MallocZone(spdlog::get("console"))
{}

MallocZone::MallocZone(std::shared_ptr<spdlog::logger> log)
 : _header(nullptr),
   _log(log)
{}

void MallocZone::reset(uint8_t *start, size_t size)
{
    internalSetBackingMemory(start, size);
    
    Block &head = *this->head();
    
    head.magic = BLOCK_MAGIC;
    head.prevOffset = 0;
    head.inuse = false;
    
    // The initial block encompasses all memory between the end of the header
    // and the end of the memory buffer.
    head.size = (uint32_t)(size - sizeof(Header));
    
    // Remember where the tail block is.
    header()->tailOffset = offsetForBlock(&head);
    
    validate(false);
}

MallocZone::Block* MallocZone::tail()
{
    Block *tail = nullptr;
    for (auto iter = begin(); iter != end(); ++iter) {
        tail = *iter;
    }
    return tail; // The block before we hit the end() iterator is the tail.
}

const MallocZone::Block* MallocZone::tail() const
{
    const Block *tail = nullptr;
    for (auto iter = cbegin(); iter != cend(); ++iter) {
        tail = *iter;
    }
    return tail; // The block before we hit the end() iterator is the tail.
}

void MallocZone::grow(uint8_t *start, size_t size)
{
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] grow:", (void *)this);
    }
    
    assert(start);
    assert(size > sizeof(Header));
    
    internalSetBackingMemory(start, size);
    
    // The assumption is that the new buffer is itself a valid zone, usually a
    // copy of the current zone with a larger size.
    assert(header()->magic == ZONE_MAGIC);
    
    // Get the tail block.
    Block *oldTail = blockForOffset(header()->tailOffset);
    
    const uint8_t *end = start + header()->size;
    const uint8_t *endOfOldTailBlock = (const uint8_t *)oldTail->data + oldTail->size;
    assert(end >= endOfOldTailBlock);
    
    if (oldTail->inuse) {
        // If the tail block not free then add a new free tail block at the end.
        
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] adding new tail block", (void *)this);
        }
        
        Block *newTail = (Block *)endOfOldTailBlock;
        const uint8_t *endOfNewTailBlock = start + size;
        newTail->magic = BLOCK_MAGIC;
        newTail->size = (uint32_t)(endOfNewTailBlock - (uint8_t *)newTail) - sizeof(Block);
        newTail->inuse = false;
        newTail->prevOffset = offsetForBlock(oldTail);
        
        header()->tailOffset = offsetForBlock(newTail);
    } else {
        // If the tail block is free then increase it's size to encompass the
        // rest of the newly enlarged buffer.
        
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] enlarging existing tail block", (void *)this);
        }
        
        const size_t deltaSize = end - endOfOldTailBlock;
        oldTail->size += deltaSize;
    }
    
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] state after grow:", (void *)this);
    }
    
    validate(VERBOSE);
    
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] finished with grow", (void *)this);
    }
}

MallocZone::Block* MallocZone::allocate(size_t size)
{
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] allocate({}):", (void *)this, size);
        dump();
    }
    
    if (size > header()->size) {
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] allocate({}) is returning NULL", (void *)this, size);
        }
        return nullptr;
    }
    
    // Blocks for allocations are always multiples of four bytes in size.
    // This ensures that blocks are always aligned on four byte boundaries
    // given that the initial block is also aligned on a four byte boundary.
    size = roundUpBlockSize(size);

    Block *best = nullptr;
    
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] searching for smallest free block that can "
                    "fit {}", (void *)this, size);
    }
    
    // Get the smallest free block that is large enough to satisfy the request.
    for (auto iter = begin(); iter != end(); ++iter) {
        Block *block = *iter;
        if (block->size >= size &&
            !block->inuse &&
            (!best || (block->size < best->size))) {
            
            best = block;
        }
    }
    
    if constexpr (VERBOSE) {
        if (best) {
            _log->trace("[zone={}] found smallest free block at {}, "
                        "size is {} bytes",
                        (void *)this, (void *)best, best->size);
        } else {
            _log->trace("[zone={}] failed to find a good free block",
                        (void*)this);
        }
    }
    
    if (best) {
        considerSplittingBlock(best, size);
        best->inuse = true;
    }
    
#ifndef NDEBUG
    if (best) {
        // Zero the contents of the new block.
        memset(best->data, 0, best->size);
    }
#endif

    if constexpr (VERBOSE) {
        _log->trace("[zone={}] allocate({}) will yield {}. state after allocate:",
                    (void *)this,
                    size,
                    (void *)best);
        dump();
    }
    
    return best;
}

void MallocZone::deallocate(Block *block)
{
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] deallocate({}):", (void *)this, (void *)block);
        dump();
    }

    if (!block) {
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] block is NULL. returning without "
                        "doing anything.", (void *)this);
        }
        return; // do nothing
    }
    
    assert(block->magic == BLOCK_MAGIC);
    assert(block->inuse);
    assert(blockIsInList(block));

    block->inuse = false;

    Block *preceding = prev(block), *following = next(block);
    
    // If the preceding chunk is free then merge this one into it. This block
    // goes away and the preceding chunk expands to fill the hole.
    if (preceding && !preceding->inuse) {
        preceding->size += block->size + sizeof(Block);
        if (following) {
            following->prevOffset = offsetForBlock(preceding);
        } else {
            // So, it turns out we removed the tail. The preceding block
            // becomes the new tail. So, let's update the header accordingly.
            assert(tail() == preceding);
            header()->tailOffset = offsetForBlock(preceding);
        }
        
        // Remove the magic tag so we can't mistake this for a valid block in
        // the future.
        block->magic = 0;
        
#ifndef NDEBUG
        // Zero the contents of the preceding block.
        // This now includes the contents of the block being deallocated.
        memset(preceding->data, 0, preceding->size);
#endif

        // Reset these pointers for a possible merge with `following', below.
        block = preceding;
        preceding = nullptr;
    }

    // If the following chunk is free then merge it into this one.
    // The following block goes away and this chunk expands to fill the hole.
    if (following && !following->inuse) {
        block->size += following->size + sizeof(Block);
        
        // Remove the magic tag so we can't mistake this for a valid block in
        // the future.
        following->magic = 0;
        
        // Update the prev offset in the block after `following' so we can
        // continue to look back.
        following = next(following);
        if (following) {
            following->prevOffset = offsetForBlock(block);
        } else {
            // So, it turns out we removed the tail. The block has become the
            // new tail. So, let's update the header accordingly.
            assert(tail() == block);
            header()->tailOffset = offsetForBlock(block);
        }
    }
    
#ifndef NDEBUG
    // Zero the contents of the block being deallocated.
    // If adjacent free blocks were merged then this also includes the contents
    // and control structures for those blocks.
    memset(block->data, 0, block->size);
#endif

    if constexpr (VERBOSE) {
        _log->trace("[zone={}] state after deallocate:", (void *)this);
        dump();
    }
}

MallocZone::Block* MallocZone::reallocate(Block *block, size_t newSize)
{
    if constexpr (VERBOSE) {
        _log->trace("[zone={}] reallocate({}, {}):",
                    (void *)this, (void *)block, newSize);
        dump();
    }
    
    if (!block) {
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] block is NULL so we're deferring to "
                        "allocate()", (void *)this);
        }
        return allocate(newSize);
    }

    assert(block->magic == BLOCK_MAGIC);
    assert(block->inuse);
    assert(blockIsInList(block));

    if (newSize == 0) {
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] newSize is zero so we're deallocating and "
                        "then allocating a minimum size block", (void *)this);
        }
        deallocate(block);
        return allocate(0);
    }

    // Blocks for allocations are always multiples of four bytes in size.
    // This ensures that blocks are always aligned on four byte boundaries
    // given that the initial block is also aligned on a four byte boundary.
    newSize = roundUpBlockSize(newSize);

    // The block is already large enough to accomodate the new size.
    // For example, the block is shrinking.
    if (block->size >= newSize) {
        considerSplittingBlock(block, newSize);
        validate(false);
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] reallocate({}) will yield the same block {}",
                        (void *)this,
                        (void *)block,
                        (void *)block);
        }
        return block;
    }

    Block *following = next(block);
    assert(!following || following->magic == BLOCK_MAGIC);

    // If this block is followed by a free block then would it satisfy our
    // requirements to take some of that free space by extending this block?
    if (following
        && !following->inuse
        && (block->size + following->size + sizeof(Block)) >= newSize) {

        // Remove the following block, extending this one so as to not leave a
        // hole in the zone.
        block->size = block->size + following->size + sizeof(Block);
        
        // Remove the magic tag from `following' so we can't mistake it for a
        // valid block in the future.
        following->magic = 0;
        
        // Update the prev offset in the new following block.
        following = next(block);
        if (following) {
            following->prevOffset = offsetForBlock(block);
        } else {
            // So, it turns out we removed the tail. The block has become the
            // new tail. So, let's update the header accordingly.
            assert(tail() == block);
            header()->tailOffset = offsetForBlock(block);
        }

        // Split the remaining free space if there's enough of it.
        considerSplittingBlock(block, newSize);

        if constexpr (VERBOSE) {
            _log->trace("[zone={}] reallocate({}) will yield {}. state after reallocate: [case where we "
                        "extend the block]",
                        (void *)this,
                        (void *)block,
                        (void *)block);
            dump();
        }
        return block;
    }

    // Can we allocate a new block of memory for the resized allocation?
    Block *newAlloc = allocate(newSize);
    if (newAlloc) {
        // Copy the contents of the old block to the new block.
        memcpy(newAlloc->data, block->data, block->size);
        deallocate(block);
        
        if constexpr (VERBOSE) {
            _log->trace("[zone={}] reallocate({}) will yield new block {}. state after reallocate: [case where we "
                        "allocate a new block]",
                        (void *)this,
                        (void *)block,
                        (void *)newAlloc);
            dump();
        }
        return newAlloc;
    }

    Block *preceding = prev(block);

    // If this block is preceded by a free block then would it satisfy our
    // requirements to merge into the preceding block?
    if (preceding
        && !preceding->inuse
        && (block->size + preceding->size + sizeof(Block)) >= newSize) {

        // Remove this block, extending the preceding one so as to not leave a
        // hole in the zone.
        preceding->inuse = true;
        preceding->size = block->size + preceding->size + sizeof(Block);
        
        if (following) {
            following->prevOffset = offsetForBlock(preceding);
        } else {
            // It looks like `block' was the tail, which is now `preceding'.
            assert(tail() == preceding);
            header()->tailOffset = offsetForBlock(preceding);
        }

        // Move the contents to the beginning of the new, combined block.
        newAlloc = preceding;
        memmove(newAlloc->data, block->data, block->size);

        // Split the remaining free space if there's enough of it.
        considerSplittingBlock(preceding, newSize);

        if constexpr (VERBOSE) {
            _log->trace("[zone={}] reallocate({}) will yield new block {}. state after reallocate: [case where we "
                        "merge with previous block]",
                        (void *)this,
                        (void *)block,
                        (void *)newAlloc);
            dump();
        }
        return newAlloc;
    }

    if constexpr (VERBOSE) {
        _log->trace("[zone={}] reallocate({}) failed and will yield NULL. state after reallocate:",
                    (void *)this,
                    (void *)block);
        dump();
    }
    return nullptr;
}

void MallocZone::validate(bool dump) const
{
    if (dump) {
        _log->trace("[zone={}] MallocZone::dump() {{", (void *)this);
    }
    
    // Check the magic number. If this is not set then the backing memory
    // region cannot be valid.
    assert(this->header()->magic == ZONE_MAGIC);
    
    int i = 0;
    const Block *prevBlock = nullptr;
    for (auto iter = begin(); iter != end(); ++iter) {
        const Block *block = *iter;
        
        assert(block);
        assert(block->magic == BLOCK_MAGIC);
        
        if (dump) {
            _log->trace("[zone={}] \t"
                        "[{}]\t{}\t{{next={}, prev={}, inuse={}, size={}}}",
                    (void *)this,
                    i,
                    (void *)block,
                    (void *)next(block),
                    (void *)prev(block),
                    block->inuse,
                    block->size);
        }
        
        // The prev pointer must actually point to the previous block.
        assert(prev(block) == prevBlock);
        
        prevBlock = *iter;
        ++i;
    }
    
    assert(tail() == blockForOffset(header()->tailOffset));
    
    if (dump) {
        _log->trace("[zone={}] }}", (void *)this);
    }
}

void MallocZone::internalSetBackingMemory(uint8_t *start, size_t size)
{
    assert(start);
    assert(size > sizeof(Header));
    assert(size < UINT32_MAX);
    
    _header = (Header *)start;
    _header->magic = ZONE_MAGIC;
    _header->size = (uint32_t)size;
    
    // Do not modify the contents of the zone. It's a design goal to be able
    // to pass in valid zone backing memory to restore a zone.
}

bool MallocZone::blockIsInList(const Block *block) const
{
    assert(pointerIsInBackingMemory(block));
    for (auto iter = begin(); iter != end(); ++iter) {
        const Block *thisBlock = *iter;
        if (thisBlock == block) {
            return true;
        }
    }
    return false;
}

void MallocZone::considerSplittingBlock(Block *block, size_t size)
{
    const size_t remainingSpace = block->size - size;
    
    // Split the block if the remaining free space is big enough.
    if (remainingSpace > MIN_SPLIT_SIZE) {
        Block *newBlock = (Block *)((uint8_t *)block + sizeof(Block) + size);
        assert((uintptr_t)newBlock % ALIGN == 0); // Block structures are always aligned.
        
        newBlock->prevOffset = offsetForBlock(block);
        newBlock->size = (uint32_t)(remainingSpace - sizeof(Block));
        newBlock->inuse = false;
        newBlock->magic = BLOCK_MAGIC;
        
        block->size = (uint32_t)size;
        
        // Update the prev offset of the next block so we can look back.
        Block *following = next(newBlock);
        if (following) {
            following->prevOffset = offsetForBlock(newBlock);
        } else {
            // Ok. This is the new tail. Update the header accordingly.
            header()->tailOffset = offsetForBlock(newBlock);
        }
        
        // If the next block is empty then merge the new free block with the
        // free block that follows it.
        following = next(newBlock);
        if (following && !following->inuse) {
            newBlock->size += following->size + sizeof(Block);
            
            // Remove the magic tag from following so we can't mistake it for a
            // valid block in the future.
            following->magic = 0;
            
            // Update the prev offset of the next block so we can look back.
            following = next(newBlock);
            if (following) {
                following->prevOffset = offsetForBlock(newBlock);
            } else {
                // Ok. This is the new tail. Update the header accordingly.
                assert(tail() == newBlock);
                header()->tailOffset = offsetForBlock(newBlock);
            }
        }
    }
}
