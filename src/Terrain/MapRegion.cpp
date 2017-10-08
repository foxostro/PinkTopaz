//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"

MapRegion::MapRegion()
 : _zoneBackingMemorySize(InitialBackingBufferSize),
   _zoneBackingMemory((uint8_t *)calloc(1, _zoneBackingMemorySize)),
   _zone(_zoneBackingMemory, _zoneBackingMemorySize)
{}

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

bool MapRegion::hasBlock(Morton3 key)
{
    return _lookup.find(key) != _lookup.end();
}

MallocZone::Block* MapRegion::getBlock(Morton3 key)
{
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        return _zone.blockForOffset(iter->second);
    } else {
        return nullptr;
    }
}

MallocZone::Block* MapRegion::getBlockAndResize(Morton3 key, size_t size)
{
    MallocZone::Block *block = nullptr;
    
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        const unsigned offset = iter->second;
        block = _zone.blockForOffset(offset);
    }
    
    block = _zone.reallocate(block, size);
    while (!block) {
        // Failed to allocate the block.
        // Resize the backing memory buffer and try again.
        _zoneBackingMemorySize *= 2;
        _zoneBackingMemory = (uint8_t *)reallocf(_zoneBackingMemory, _zoneBackingMemorySize);
        _zone.grow(_zoneBackingMemory, _zoneBackingMemorySize);
        
        // Try again.
        block = _zone.reallocate(block, size);
    }
    
    _lookup[key] = _zone.offsetForBlock(block);
    
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
    
    _lookup[key] = _zone.offsetForBlock(block);
}

boost::optional<std::vector<uint8_t>>
MapRegion::getChunkBytesFromStash(Morton3 key)
{
    if (!hasBlock(key)) {
        return boost::none;
    }
    
    MallocZone::Block *block = getBlock(key);
    const size_t size = block->size;
    
    std::vector<uint8_t> bytes(size);
    assert(bytes.size() >= size);
    memcpy(&bytes[0], block->data, size);
    
    return boost::make_optional(bytes);
}
