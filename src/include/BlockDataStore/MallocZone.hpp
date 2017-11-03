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
#include <cassert>

// Malloc-like allocator. This allocates and deallocates blocks of memory in a
// provided memory buffer. The heap tracking information contains no raw
// pointers and is transparently relocatable.
//
// It is a design goal to be able to persistently store a MallocZone in a
// memory mapped file.
class MallocZone
{
public:
    static constexpr uint32_t ZONE_MAGIC = 'enoz';
    static constexpr uint32_t BLOCK_MAGIC = 'kolb';
    
    // A memory allocation within the zone.
    struct Block
    {
        // Magic number. For a valid block, this is set to BLOCK_MAGIC.
        // This is a very simple way to check for some types of corruption.
        uint32_t magic;
        
        // The offset (in number of bytes) of the previous block from the
        // start of the zone.
        // The head of the list has 0 for `prevOffset'.
        uint32_t prevOffset;
        
        // 1 if the block is being used, 0 if free.
        uint32_t inuse;
        
        // The number of bytes in `data'.
        uint32_t size;
        
        // The memory buffer associated with the block.
        uint8_t data[0];
        
        Block() : magic(BLOCK_MAGIC), prevOffset(0), inuse(0), size(0) {}
    };
    
    // Header for the zone. Written to the beginning of the zone.
    struct Header
    {
        // Magic number. For a valid zone, this is set to ZONE_MAGIC.
        // This is a very simple way to check for some types of corruption.
        uint32_t magic;
        
        // Size of the backing memory region.
        uint32_t size;
        
        // The offset (in number of bytes) of the tail block from the start of
        // the zone.
        uint32_t tailOffset;
        
        // The region of memory which follows the header is a list of Block
        // structures, all placed adjacent to one another in memory.
        Block head;
    };
    
    template<typename Z, typename T>
    class Iterator
    {
    public:
        // No default constructor.
        Iterator() = delete;
        
        // Constructor. Accepts a reference to the zone and the current block.
        // The current block in `curr' must be a valid block in the specified
        // zone, or else nullptr.
        Iterator(Z *zone, T *curr)
         : _zone(zone), _curr(curr)
        {
            assert(zone);
            assert(!curr || zone->pointerIsInBackingMemory(curr));
        }
        
        // Copy constructor.
        Iterator(const Iterator<Z, T> &a) = default;
        
        // Pre-Increment to the next block.
        // The block is set to nullptr at the tail.
        Iterator<Z, T>& operator++()
        {
            assert(_zone);
            assert(_curr);
            _curr = _zone->next(_curr);
            assert(!_curr || _curr->magic == BLOCK_MAGIC);
            return *this;
        }
        
        // Post-Increment to the next block.
        // The block is set to nullptr at the tail.
        Iterator<Z, T> operator++(int)
        {
            Iterator<Z, T> temp(*this);
            operator++();
            return temp;
        }
        
        T* operator*() const
        {
            return _curr;
        }
        
        T* operator->() const
        {
            return _curr;
        }
        
        // Equality operator.
        bool operator==(const Iterator<Z, T> &a) const
        {
            return _zone == a._zone && _curr == a._curr;
        }
        
        // Not-equality operator.
        bool operator!=(const Iterator<Z, T> &a) const
        {
            return !(*this == a);
        }
        
    private:
        Z *_zone;
        T *_curr;
    };
    
    using BlockIterator = Iterator<MallocZone, Block>;
    using ConstBlockIterator = Iterator<const MallocZone, const Block>;
    
    // Default destructor.
    ~MallocZone() = default;
    
    // Default constructor. The zone begins with no backing memory buffer.
    // A call to grow() is necessary before any allocations may be made.
    MallocZone();
    
    // No copy constructor.
    MallocZone(const MallocZone &) = delete;
    
    // Reset the zone so it is entirely free.
    void reset(uint8_t *start, size_t size);
    
    // Gets the zone header.
    inline Header* header()
    {
        assert(_header);
        return _header;
    }
    
    // Gets the zone header.
    inline const Header* header() const
    {
        assert(_header);
        return _header;
    }

    // Gets the head of the block list.
    inline Block* head() {
        return &(header()->head);
    }
    
    // Gets the head of the block list.
    inline const Block* head() const {
        return &(header()->head);
    }
    
    // Scans the list to find the tail of the block list.
    Block* tail();
    
    // Scans the list to find the tail of the block list.
    const Block* tail() const;
    
    // Returns true if the specified address is in the backing memory region.
    inline bool pointerIsInBackingMemory(const uint8_t *ptr) const {
        assert(ptr);
        const size_t size = header()->size;
        const uint8_t *start = (uint8_t *)header();
        const uint8_t *end = start + size;
        return ptr >= start && ptr < end;
    }
    
    // Returns true if the specified address is in the backing memory region.
    inline bool pointerIsInBackingMemory(const Block *block) const {
        return pointerIsInBackingMemory((const uint8_t *)block);
    }
    
    // Returns true if the specified address is in the backing memory region
    // or just after at the end of the buffer.
    inline bool pointerIsInBackingMemoryOrJustAfter(const uint8_t *ptr) const {
        assert(ptr);
        const size_t size = header()->size;
        const uint8_t *start = (uint8_t *)header();
        const uint8_t *end = start + size;
        return ptr >= start && ptr <= end;
    }
    
    // Returns true if the specified address is in the backing memory region
    // or just after at the end of the buffer.
    inline bool pointerIsInBackingMemoryOrJustAfter(const Block *block) const {
        return pointerIsInBackingMemoryOrJustAfter((const uint8_t *)block);
    }
    
    // Gets the next block after the one specified.
    // Returns nullptr if the specified block is the tail.
    inline Block* next(const Block *block)
    {
        assert(pointerIsInBackingMemory(block));
        Block *nextBlock = (Block *)(block->data + block->size);
        assert(pointerIsInBackingMemoryOrJustAfter(nextBlock));
        if (pointerIsInBackingMemory(nextBlock)) {
            return nextBlock;
        } else {
            return nullptr;
        }
    }
    
    // Gets the next block after the one specified.
    // Returns nullptr if the specified block is the tail.
    inline const Block* next(const Block *block) const
    {
        assert(pointerIsInBackingMemory(block));
        const Block *nextBlock = (const Block *)(block->data + block->size);
        assert(pointerIsInBackingMemoryOrJustAfter(nextBlock));
        if (pointerIsInBackingMemory(nextBlock)) {
            return nextBlock;
        } else {
            return nullptr;
        }
    }
    
    // Gets the block previous to the one specified.
    // Returns nullptr if the specified block is the head.
    inline Block* prev(const Block *block)
    {
        assert(pointerIsInBackingMemory(block));
        const unsigned offset = block->prevOffset;
        if (offset == 0) {
            return nullptr;
        } else {
            uint8_t *start = (uint8_t *)header();
            Block *prevBlock = (Block *)(start + offset);
            assert(pointerIsInBackingMemory(prevBlock));
            return prevBlock;
        }
    }
    
    // Gets the block previous to the one specified.
    // Returns nullptr if the specified block is the head.
    inline const Block* prev(const Block *block) const
    {
        assert(pointerIsInBackingMemory(block));
        const unsigned offset = block->prevOffset;
        if (offset == 0) {
            return nullptr;
        } else {
            const uint8_t *start = (const uint8_t *)header();
            const Block *prevBlock = (const Block *)(start + offset);
            assert(pointerIsInBackingMemory(prevBlock));
            return prevBlock;
        }
    }
    
    // Gets the offset of the specified block from the start of the zone.
    inline unsigned offsetForBlock(const Block *block) const
    {
        assert(pointerIsInBackingMemory(block));
        const uint8_t *start = (const uint8_t *)header();
        const uint8_t *pointer = (const uint8_t *)block;
        return (unsigned)(pointer - start);
    }
    
    // Gets the block associated with the specified offset.
    inline Block* blockForOffset(unsigned offset)
    {
        uint8_t *start = (uint8_t *)header();
        Block *block = (Block *)(start + offset);
        assert(blockIsInList(block));
        return block;
    }
    
    // Gets the block associated with the specified offset.
    inline const Block* blockForOffset(unsigned offset) const
    {
        const uint8_t *start = (const uint8_t *)header();
        const Block *block = (const Block *)(start + offset);
        assert(blockIsInList(block));
        return block;
    }
    
    // Gets an iterator pointing to the head block of the list.
    inline BlockIterator begin()
    {
        return BlockIterator(this, &(header()->head));
    }
    
    // Gets an iterator pointing to the head block of the list.
    inline ConstBlockIterator begin() const
    {
        const Block *head = (const Block *)&(header()->head);
        return ConstBlockIterator(this, head);
    }
    
    // Gets an iterator pointing to the head block of the list.
    inline ConstBlockIterator cbegin() const
    {
        return begin();
    }
    
    // Gets an iterator representing the end of the list.
    // This is not the tail. Taking an iterator to the tail and incrementing it
    // will result in this iterator.
    inline BlockIterator end()
    {
        return BlockIterator(this, nullptr);
    }
    
    // Gets an iterator representing the end of the list.
    // This is not the tail. Taking an iterator to the tail and incrementing it
    // will result in this iterator.
    inline ConstBlockIterator end() const
    {
        return ConstBlockIterator(this, nullptr);
    }
    
    // Gets an iterator representing the end of the list.
    // This is not the tail. Taking an iterator to the tail and incrementing it
    // will result in this iterator.
    inline ConstBlockIterator cend() const
    {
        return end();
    }
    
    // Increase the size of the backing memory buffer.
    // The new buffer must itself be a valid zone backing buffer.
    // Note that you must not free the old backing buffer until the call to
    // grow() has completed.
    void grow(uint8_t *start, size_t size);
    
    // Allocates a block of memory of the given size.
    // May return nullptr if the request cannot be satisfied.
    // If size is zero a new minimum-sized object is allocated.
    Block* allocate(size_t size);
    
    // Deallocates a memory allocation pointed to by `block.'
    // If `block' is nullptr then no operation is performed.
    void deallocate(Block *block);
    
    // Tries to change the size of the allocation to size, and returns a
    // pointer to the new block. This may move the allocation to a different
    // part of the heap, copying as much of the old data as will fit in the new
    // allocation, and freeing the old allocation.
    //
    // When extending an allocation, no guarantee is made as to the value of
    // memory in the extended region.
    //
    // If block is nullptr, reallocate() is identical to a call to allocate().
    //
    // If size is zero and block is not nullptr, a new minimum-sized object is
    // allocated and the original object is freed.
    Block* reallocate(Block *block, size_t newSize);
    
    // Returns true if the block could be found in the list, false otherwise.
    bool blockIsInList(const Block *block) const;
    
    // Print allocations to the log for debugging purposes.
    void validate(bool dump) const;
    inline void dump() const
    {
        validate(true);
        
    }
    
private:
    Header *_header;
    
    void internalSetBackingMemory(uint8_t *start, size_t size);
    void considerSplittingBlock(MallocZone::Block *block, size_t size);
};

#endif /* MallocZone_hpp */
