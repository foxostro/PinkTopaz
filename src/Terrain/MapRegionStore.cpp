//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"
#include "FileUtilities.hpp"
#include "SDL.h" // for SDL_Log
#include <boost/filesystem.hpp>

static constexpr size_t initialBackingBufferSize = 1024*1024*1024;

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

uint8_t* MapRegionStore::getBlock(Morton3 key)
{
    uint8_t *block = nullptr;
    
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        const ptrdiff_t offset = iter->second;
        block = _zone.getPointerFromOffset(offset);
    }
    
    if (block) {
        _lookup[key] = _zone.getOffsetFromPointer(block);
    }
    
    return block;
}

uint8_t* MapRegionStore::getBlockAndResize(Morton3 key, size_t size)
{
    uint8_t *block = nullptr;
    
    if (auto iter = _lookup.find(key); iter != _lookup.end()) {
        const ptrdiff_t offset = iter->second;
        block = _zone.getPointerFromOffset(offset);
    }
    
    SDL_Log("reallocating the block for size %zu", size);
    size_t count = 0;
    block = _zone.reallocate(block, size);
    while (!block) {
        // Failed to allocate the block.
        // Resize the backing memory buffer and try again.
        const size_t newSize = _zoneBackingMemorySize * 2;
        SDL_Log("[%zu] Resizing backing memory buffer from %zu to %zu",
                count, _zoneBackingMemorySize, newSize);
        assert(newSize < 1073741824);
        _zoneBackingMemory = (uint8_t *)realloc(_zoneBackingMemory, newSize);
        _zoneBackingMemorySize = newSize;
        _zone.setBackingMemory(_zoneBackingMemory, _zoneBackingMemorySize);
        block = _zone.reallocate(block, size);
        ++count;
    }
    SDL_Log("[%zu] got the block: %p", count, block);
    
    _lookup[key] = _zone.getOffsetFromPointer(block);
    
    return block;
}

void MapRegionStore::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
{
    const size_t size = bytes.size();
    uint8_t *block = getBlockAndResize(key, size);
    memcpy(block, &bytes[0], size);
    _lookup[key] = _zone.getOffsetFromPointer(block);
}

boost::optional<std::vector<uint8_t>> MapRegionStore::getChunkBytesFromStash(Morton3 key)
{
    if (!hasBlock(key)) {
        return boost::none;
    }
    
    uint8_t *block = getBlock(key);
    size_t size = _zone.getBlockSize(block);
    
    std::vector<uint8_t> bytes(size);
    assert(bytes.size() >= size);
    memcpy(&bytes[0], block, size);
    
    return boost::make_optional(bytes);
}
