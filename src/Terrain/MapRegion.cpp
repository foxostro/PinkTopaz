//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"

MapRegion::~MapRegion()
{
    free(_zoneBackingMemory);
}

MapRegion::MapRegion()
 : _zoneBackingMemorySize(InitialBackingBufferSize),
   _zoneBackingMemory((uint8_t *)calloc(1, _zoneBackingMemorySize)),
   _zone(_zoneBackingMemory, _zoneBackingMemorySize),
   _lookupTableOffset(0)
{
    if (!_zoneBackingMemory) {
        throw Exception("Out of Memory");
    }
}

boost::optional<Array3D<Voxel>> MapRegion::load(const AABB &bbox, Morton3 key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (const auto maybeBytes = getChunkBytesFromStash(key); maybeBytes) {
        const auto &bytes = *maybeBytes;
        auto chunk = _serializer.load(bbox, bytes);
        return boost::make_optional(chunk);
    } else {
        return boost::none;
    }
}

void MapRegion::store(Morton3 key, const Array3D<Voxel> &voxels)
{
    std::lock_guard<std::mutex> lock(_mutex);
    stashChunkBytes(key, _serializer.store(voxels));
}

void MapRegion::growBackingMemory()
{
    _zoneBackingMemorySize *= 2;
    _zoneBackingMemory = (uint8_t *)reallocf(_zoneBackingMemory, _zoneBackingMemorySize);
    if (!_zoneBackingMemory) {
        throw Exception("Out of Memory");
    }
    _zone.grow(_zoneBackingMemory, _zoneBackingMemorySize);
}

void MapRegion::growLookupTable(size_t newCapacity)
{
    const size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
    MallocZone::Block *newBlock;
    while (!(newBlock = _zone.reallocate(lookupTableBlock(), newSize))) {
        growBackingMemory();
    }
    _lookupTableOffset = _zone.offsetForBlock(newBlock);
}

MallocZone::Block* MapRegion::getBlock(Morton3 key)
{
    if (auto maybeOffset = loadOffsetForKey(key); maybeOffset) {
        return _zone.blockForOffset(*maybeOffset);
    }
    return nullptr;
}

MallocZone::Block* MapRegion::getBlockAndResize(Morton3 key, size_t size)
{
    MallocZone::Block *block = getBlock(key);
    unsigned offset;
    
    if (block) {
        offset = _zone.offsetForBlock(block);
        
        while (!(block = _zone.reallocate(_zone.blockForOffset(offset), size))){
            growBackingMemory();
        }
    } else {
        while (!(block = _zone.allocate(size))) {
            growBackingMemory();
        }
    }
    
    offset = _zone.offsetForBlock(block);
    storeOffsetForKey(key, offset); // storeOffsetForKey may invalidate `block'.
    block = _zone.blockForOffset(offset);
    return block;
}

void MapRegion::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
{
    const size_t size = bytes.size();
    assert(size > 0);
    MallocZone::Block *block = getBlockAndResize(key, size);
    memcpy(block->data, &bytes[0], size);
    memset(block->data + size, 0, block->size - size);
}

boost::optional<std::vector<uint8_t>>
MapRegion::getChunkBytesFromStash(Morton3 key)
{
    if (!loadOffsetForKey(key)) {
        return boost::none;
    }
    
    MallocZone::Block *block = getBlock(key);
    const size_t size = block->size;
    
    std::vector<uint8_t> bytes(size);
    assert(bytes.size() >= size);
    memcpy(&bytes[0], block->data, size);
    
    return boost::make_optional(bytes);
}

void MapRegion::storeOffsetForKey(Morton3 mortonKey, uint32_t offset)
{
    uint64_t key = (size_t)mortonKey;
    
    if (!lookupTableBlock()) {
        growLookupTable(InitialLookTableCapacity);
        lookup().capacity = InitialLookTableCapacity;
        lookup().numberOfEntries = 0;
    }
    
    // Update an existing entry, if there is one.
    for (size_t i = 0, n = lookup().numberOfEntries; i < n; ++i) {
        auto &entry = lookup().entries[i];
        if (key == entry.key) {
            entry.offset = offset;
            return;
        }
    }
    
    // Grow the lookup table, if necessary.
    if (lookup().numberOfEntries >= lookup().capacity) {
        size_t newCapacity = lookup().capacity * 2;
        growLookupTable(newCapacity);
        lookup().capacity = newCapacity;
    }
    
    // Add a new entry at the end of the table.
    lookup().entries[lookup().numberOfEntries++] = {key, offset};
}

boost::optional<uint32_t> MapRegion::loadOffsetForKey(Morton3 mortonKey)
{    
    if (lookupTableBlock()) {
        uint64_t key = (size_t)mortonKey;
        for (size_t i = 0, n = lookup().numberOfEntries; i < n; ++i) {
            auto &entry = lookup().entries[i];
            if (key == entry.key) {
                return entry.offset;
            }
        }
    }
    return boost::none;
}
