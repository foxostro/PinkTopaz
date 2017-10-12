//
//  BoxedMallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#ifndef BoxedMallocZone_hpp
#define BoxedMallocZone_hpp

#include "MallocZone.hpp"
#include <cstdint>
#include <limits>

// Wraps a MallocZone to provide an API which does not expose raw pointers.
// Instead, access is only provided through BlockPointer, which uses the offset
// so that they are not invalidated when the zone is resized.
class BoxedMallocZone
{
public:
    using Offset = uint32_t;
    static constexpr Offset NullOffset = std::numeric_limits<uint32_t>::max();
    
    class BlockPointer
    {
        BoxedMallocZone &_zone;
        Offset _offset;

    public:
        ~BlockPointer() = default;
        
        BlockPointer() = delete;
        
        BlockPointer(BoxedMallocZone &zone)
         : _zone(zone), _offset(NullOffset)
        {}
        
        BlockPointer(BoxedMallocZone &zone, Offset offset)
         : _zone(zone), _offset(offset)
        {
            assert(_offset != 0);
        }
        
        BlockPointer(const BlockPointer &ptr) = delete;
        
        BlockPointer(BlockPointer &&ptr)
         : _zone(ptr._zone),
           _offset(ptr._offset)
        {
            assert(_offset != 0);
            ptr._offset = NullOffset;
        }
        
        BlockPointer& operator=(BlockPointer &&ptr)
        {
            assert(&_zone == &ptr._zone);
            _offset = ptr._offset;
            ptr._offset = NullOffset;
            return *this;
        }
        
        explicit operator bool() const
        {
            return _offset != NullOffset;
        }
        
        MallocZone::Block* operator*()
        {
            return _zone.blockForOffset(getOffset());
        }
        
        const MallocZone::Block* operator*() const
        {
            return _zone.blockForOffset(getOffset());
        }
        
        MallocZone::Block* operator->()
        {
            return _zone.blockForOffset(getOffset());
        }
        
        const MallocZone::Block* operator->() const
        {
            return _zone.blockForOffset(getOffset());
        }
        
        Offset getOffset() const
        {
            return _offset;
        }
    };
    
    // Destructor.
    ~BoxedMallocZone();
    
    // Default constructor. The zone begins with no backing memory buffer.
    // A call to grow() is necessary before any allocations may be made.
    BoxedMallocZone();
    
    // No copy constructor.
    BoxedMallocZone(const BoxedMallocZone &) = delete;
    
    // Reset the zone so it is entirely free.
    void reset(uint8_t *start, size_t size);
    
    // Gets the block associated with the specified offset.
    BlockPointer blockPointerForOffset(Offset offset);
    
    // Increase the size of the backing memory buffer.
    // The new buffer must itself be a valid zone backing buffer.
    void grow(uint8_t *start, size_t size);
    
    // Allocates a block of memory of the given size.
    // May return a Null object if the request cannot be satisfied.
    // If size is zero a new minimum-sized object is allocated.
    BlockPointer allocate(size_t size);
    
    // Deallocates a memory allocation refered to by `block'.
    // If the block is a Null object then no operation is performed.
    void deallocate(BlockPointer &&block);
    
    // Tries to change the size of the allocation to size, and returns a
    // BlockPointer referring to the new allocation. This may move the
    // allocation to a different part of the heap, copying as much of the old
    // data as will fit in the new allocation, and freeing the old allocation.
    //
    // When extending an allocation, no guarantee is made as to the value of
    // memory in the extended region.
    //
    // If the offset is NullOffset then reallocate() is identical to a call to
    // allocate().
    //
    // If size is zero and the block is not a Null object then a new minimum-
    // sized object is allocated and the original object is freed.
    BlockPointer reallocate(Offset offset, size_t newSize);
    
private:
    MallocZone _zone;
    
    // Gets the block associated with the specified offset.
    MallocZone::Block* blockForOffset(Offset offset);
};

#endif /* BoxedMallocZone_hpp */
