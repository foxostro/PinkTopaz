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
    free(_lookup);
    free(_zoneBackingMemory);
}

MapRegion::MapRegion()
 : _zoneBackingMemorySize(InitialBackingBufferSize),
   _zoneBackingMemory((uint8_t *)calloc(1, _zoneBackingMemorySize)),
   _zone(_zoneBackingMemory, _zoneBackingMemorySize),
   _lookup(nullptr)
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
    
    block = _zone.reallocate(block, size);
    while (!block) {
        // Failed to allocate the block.
        // Resize the backing memory buffer and try again.
        _zoneBackingMemorySize *= 2;
        _zoneBackingMemory = (uint8_t *)reallocf(_zoneBackingMemory, _zoneBackingMemorySize);
        if (!_zoneBackingMemory) {
            throw Exception("Out of Memory");
        }
        _zone.grow(_zoneBackingMemory, _zoneBackingMemorySize);
        
        // Try again.
        block = _zone.reallocate(block, size);
    }
    
    storeOffsetForKey(key, _zone.offsetForBlock(block));
    
    return block;
}

void MapRegion::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
{
    const size_t size = bytes.size();
    assert(size > 0);
    
    MallocZone::Block *block = getBlockAndResize(key, size);
    
    // Copy the bytes into the block.
    memcpy(block->data, &bytes[0], size);
    
    // Any remaining space in the block is zeroed.
    memset(block->data + size, 0, block->size - size);
    
    storeOffsetForKey(key, _zone.offsetForBlock(block));
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
    
    if (!_lookup) {
        const size_t capacity = 1024;
        _lookup = (LookupTable *)malloc(sizeof(LookupTable) + sizeof(LookUpTableEntry)*capacity);
        if (!_lookup) {
            throw Exception("Out of Memory");
        }
        _lookup->capacity = capacity;
        _lookup->numberOfEntries = 0;
    }
    
    // Update an existing entry, if there is one.
    for (size_t i = 0, n = _lookup->numberOfEntries; i < n; ++i) {
        auto &entry = _lookup->entries[i];
        if (key == entry.key) {
            entry.offset = offset;
            return;
        }
    }
    
    // Grow the lookup table, if necessary.
    if (_lookup->numberOfEntries >= _lookup->capacity) {
        size_t newCapacity = _lookup->capacity * 2;
        size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
        _lookup = (LookupTable *)reallocf((void *)_lookup, newSize);
        if (!_lookup) {
            throw Exception("Out of Memory");
        }
    }
    
    // Add a new entry at the end of the table.
    _lookup->entries[_lookup->numberOfEntries++] = {key, offset};
}

boost::optional<uint32_t> MapRegion::loadOffsetForKey(Morton3 mortonKey)
{    
    if (_lookup) {
        uint64_t key = (size_t)mortonKey;
        for (size_t i = 0, n = _lookup->numberOfEntries; i < n; ++i) {
            auto &entry = _lookup->entries[i];
            if (key == entry.key) {
                return entry.offset;
            }
        }
    }
    return boost::none;
}
