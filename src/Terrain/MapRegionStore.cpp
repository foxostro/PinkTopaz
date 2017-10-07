//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"
#include "FileUtilities.hpp"
#include <boost/filesystem.hpp>

static constexpr size_t initialBackingBufferSize = 256;

static boost::filesystem::path fileNameForChunk(Morton3 key)
{
    const std::string fileName("Chunk_" + std::to_string((size_t)key) + ".bin");
    return getPrefPath() / fileName;
}

MapRegionStore::MapRegionStore(const AABB &boundingBox, unsigned mapRegionSize)
: GridIndexer(boundingBox, glm::ivec3(boundingBox.extent*2.f) / (int)mapRegionSize),
  _zoneBackingMemorySize(initialBackingBufferSize),
  _zoneBackingMemory((uint8_t *)calloc(1, _zoneBackingMemorySize)),
  _zone(_zoneBackingMemory, _zoneBackingMemorySize)
{}

boost::optional<Array3D<Voxel>> MapRegionStore::load(const AABB &boundingBox, Morton3 key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    const auto maybeBytes = binaryFileContents(fileNameForChunk(key));
    if (maybeBytes) {
        const auto &bytes = *maybeBytes;
        stashChunkBytes(key, bytes);
        auto chunk = _serializer.load(boundingBox, bytes);
        return boost::make_optional(chunk);
    } else {
        return boost::none;
    }
}

void MapRegionStore::store(const AABB &boundingBox, Morton3 key, const Array3D<Voxel> &voxels)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    const auto bytes = _serializer.store(voxels);
    auto path = fileNameForChunk(key);
    saveBinaryFile(path, bytes);
    stashChunkBytes(key, bytes);
}

bool MapRegionStore::hasBlock(Morton3 key)
{
    auto iter = _lookup.find(key);
    return iter != _lookup.end();
}

MallocZone::Block* MapRegionStore::getBlock(Morton3 key)
{
    MallocZone::Block *block = nullptr;
    
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        unsigned offset = iter->second;
        block = _zone.blockForOffset(offset);
    }
    
    return block;
}

MallocZone::Block* MapRegionStore::getBlockAndResize(Morton3 key, size_t size)
{
    MallocZone::Block *block = nullptr;
    
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        const unsigned offset = iter->second;
        block = _zone.blockForOffset(offset);
    }
    
    size_t count = 0;
    block = _zone.reallocate(block, size);
    while (!block) {
        // Failed to allocate the block.
        // Resize the backing memory buffer and try again.
        const size_t newSize = _zoneBackingMemorySize * 2;
        assert(newSize < 1024*1024*1024);
        
        // Create a new buffer with the contents of the old buffer, but bigger.
        uint8_t *newBuffer = (uint8_t *)calloc(1, newSize);
        memcpy(newBuffer, _zoneBackingMemory, _zoneBackingMemorySize);
        assert(newBuffer);
        
        _zone.grow(newBuffer, newSize);
        
        // We can't free the old buffer until after the call to grow().
        free(_zoneBackingMemory);
        _zoneBackingMemory = newBuffer;
        _zoneBackingMemorySize = newSize;
        
        block = _zone.reallocate(block, size);
        ++count;
    }
    
    _lookup[key] = _zone.offsetForBlock(block);
    
    return block;
}

void MapRegionStore::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
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

boost::optional<std::vector<uint8_t>> MapRegionStore::getChunkBytesFromStash(Morton3 key)
{
    if (!hasBlock(key)) {
        return boost::none;
    }
    
    MallocZone::Block *block = getBlock(key);
    const size_t size = block->size;
    
    std::vector<uint8_t> bytes(size);
    assert(bytes.size() >= size);
    memcpy(&bytes[0], block, size);
    
    return boost::make_optional(bytes);
}
