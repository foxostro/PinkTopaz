//
//  ManagedMallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#include "Malloc/ManagedMallocZone.hpp"
#include "Exception.hpp"

ManagedMallocZone::ManagedMallocZone(const boost::filesystem::path &fileName,
                                     size_t initialFileSize)
 : _file(fileName)
{
    mapFile(initialFileSize);
}

BoxedMallocZone::BoxedBlock ManagedMallocZone::blockPointerForOffset(BoxedMallocZone::Offset offset)
{
    return _zone.blockPointerForOffset(offset);
}

BoxedMallocZone::BoxedBlock ManagedMallocZone::allocate(size_t size)
{
    while (true) {
        auto block = _zone.allocate(size);
        
        if (block) {
            return block;
        } else {
            growBackingMemory();
        }
    }
}

void ManagedMallocZone::deallocate(BoxedMallocZone::BoxedBlock &&block)
{
    _zone.deallocate(std::move(block));
}

BoxedMallocZone::BoxedBlock ManagedMallocZone::reallocate(BoxedMallocZone::BoxedBlock &&block, size_t newSize)
{
    if (!block) {
        return allocate(newSize);
    }
    
    while (true) {
        auto newBlock = _zone.reallocate(block.getOffset(), newSize);
        
        if (newBlock) {
            return newBlock;
        } else {
            growBackingMemory();
        }
    }
}

void ManagedMallocZone::mapFile(size_t minimumFileSize)
{
    _file.unmapFile();
    bool mustReset = _file.mapFile(minimumFileSize);
    size_t newZoneSize = _file.size() - sizeof(Header);
    
    if (mustReset) {
        header()->magic = MANAGED_MALLOC_ZONE_MAGIC;
        header()->version = MANAGED_MALLOC_ZONE_VERSION;
        header()->zoneSize = newZoneSize;
        header()->lookupTableOffset = BoxedMallocZone::NullOffset;
        _zone.reset(header()->zoneData, header()->zoneSize);
    } else {
        if (header()->magic != MANAGED_MALLOC_ZONE_MAGIC) {
            throw Exception("Unexpected magic number in managed file: found %d but expected %d", header()->magic, MANAGED_MALLOC_ZONE_MAGIC);
        }
        
        if (header()->version != MANAGED_MALLOC_ZONE_VERSION) {
            throw Exception("Unexpected version number in managed file: found %d but expected %d", header()->version, MANAGED_MALLOC_ZONE_VERSION);
        }
        
        _zone.grow(header()->zoneData, newZoneSize);
    }
}

void ManagedMallocZone::growBackingMemory()
{
    mapFile(_file.size() * 2);
}
