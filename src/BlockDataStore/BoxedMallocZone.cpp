//
//  BoxedMallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#include "BlockDataStore/BoxedMallocZone.hpp"

BoxedMallocZone::~BoxedMallocZone() = default;

BoxedMallocZone::BoxedMallocZone() = default;

void BoxedMallocZone::reset(uint8_t *start, size_t size)
{
    _zone.reset(start, size);
}

BoxedMallocZone::BoxedBlock
BoxedMallocZone::blockPointerForOffset(Offset offset)
{
    return BoxedBlock(*this, offset);
}

void BoxedMallocZone::grow(uint8_t *start, size_t size)
{
    _zone.grow(start, size);
}

BoxedMallocZone::BoxedBlock BoxedMallocZone::allocate(size_t size)
{
    MallocZone::Block *block = _zone.allocate(size);
    Offset offset;
    if (block) {
        offset = _zone.offsetForBlock(block);
    } else {
        offset = NullOffset;
    }
    return BoxedBlock(*this, offset);
}

void BoxedMallocZone::deallocate(BoxedBlock &&ptr)
{
    MallocZone::Block *block = blockForOffset(ptr.getOffset());
    _zone.deallocate(block);
}

BoxedMallocZone::BoxedBlock
BoxedMallocZone::reallocate(Offset offsetOfOldBlock, size_t newSize)
{
    MallocZone::Block *oldBlock = blockForOffset(offsetOfOldBlock);
    MallocZone::Block *newBlock = _zone.reallocate(oldBlock, newSize);
    Offset offset;
    if (newBlock) {
        offset = _zone.offsetForBlock(newBlock);
    } else {
        offset = NullOffset;
    }
    return BoxedBlock(*this, offset);
}

MallocZone::Block* BoxedMallocZone::blockForOffset(Offset offset)
{
    if (offset == NullOffset) {
        return nullptr;
    } else {
        return _zone.blockForOffset(offset);
    }
}
