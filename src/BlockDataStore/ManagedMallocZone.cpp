//
//  ManagedMallocZone.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#include "BlockDataStore/ManagedMallocZone.hpp"
#include "Exception.hpp"

ManagedMallocZone::ManagedMallocZone(const boost::filesystem::path &fileName,
                                     size_t initialFileSize,
                                     uint32_t magic, uint32_t version)
 : _magic(magic), _version(version), _file(fileName)
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
        header()->magic = _magic;
        header()->version = _version;
        header()->zoneSize = newZoneSize;
        header()->lookupTableOffset = BoxedMallocZone::NullOffset;
        _zone.reset(header()->zoneData, header()->zoneSize);
    } else {
        if (header()->magic != _magic) {
            throw Exception("Unexpected magic number in managed file: found %d but expected %d", header()->magic, _magic);
        }
        
        if (header()->version != _version) {
            throw Exception("Unexpected version number in managed file: found %d but expected %d", header()->version, _version);
        }
        
        _zone.grow(header()->zoneData, newZoneSize);
    }
}

void ManagedMallocZone::growBackingMemory()
{
    mapFile(_file.size() * 2);
}
