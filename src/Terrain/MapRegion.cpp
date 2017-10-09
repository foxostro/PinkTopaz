//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"
#include "SDL.h" // for SDL_Log
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

MapRegion::~MapRegion()
{
    unmapFile();
}

MapRegion::MapRegion(const boost::filesystem::path &regionFileName)
 : _regionFileName(regionFileName),
   _fd(0),
   _backingMemorySize(0),
   _backingMemory(nullptr)
{
    mapFile(InitialBackingBufferSize);
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

void MapRegion::mapFile(size_t minimumFileSize)
{
    bool mustReset = false;
    
    unmapFile();
    
//    SDL_Log("[%p] mapping file: %s", this, _regionFileName.c_str());
    
    if ((_fd = open(_regionFileName.c_str(), O_RDWR | O_CREAT, 0)) < 0) {
        throw Exception("Failed to open file: %s with errno=%d", _regionFileName.c_str(), errno);
    }
    
    if (fchmod(_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
        throw Exception("Failed to chmod file: %s with errno=%d", _regionFileName.c_str(), errno);
    }
    
    off_t fileSize;
    {
        struct stat s;
        if (fstat(_fd, &s) < 0) {
            close(_fd);
            throw Exception("Failed to stat file: %s with errno=%d", _regionFileName.c_str(), errno);
        }
        fileSize = s.st_size;
    }
    
    if (fileSize == 0) {
        mustReset = true;
    }
    
    if (fileSize < minimumFileSize) {
        if (ftruncate(_fd, minimumFileSize) < 0) {
            close(_fd);
            throw Exception("Failed to resize file: %s with errno=%d", _regionFileName.c_str(), errno);
        }
        fileSize = minimumFileSize;
    }
    
    _backingMemorySize = fileSize;
    _backingMemory = (uint8_t *)mmap(nullptr,
                                     fileSize,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED,
                                     _fd,
                                     0);
    
    if (MAP_FAILED == _backingMemory) {
        close(_fd);
        throw Exception("Failed to map file: %s with errno=%d", _regionFileName.c_str(), errno);
    }
    
    if (mustReset) {
//        SDL_Log("[%p] resetting zone for file: %s", this, _regionFileName.c_str());
        header()->magic = MAP_REGION_MAGIC;
        header()->zoneSize = fileSize - sizeof(Header);
        _zone.reset(header()->zoneData, header()->zoneSize);
    } else {
        if (header()->magic != MAP_REGION_MAGIC) {
            throw Exception("Unexpected magic number in map region file: found %d but expected %d", header()->magic, MAP_REGION_MAGIC);
        }
        _zone.grow(header()->zoneData, header()->zoneSize);
    }
}

void MapRegion::unmapFile()
{
    if (_backingMemory) {
//        SDL_Log("[%p] unmapping file: %s", this, _regionFileName.c_str());
        
        if (munmap(_backingMemory, _backingMemorySize) < 0) {
            SDL_Log("Failed to unmap file.");
        }
        
        _backingMemory = nullptr;
        _backingMemorySize = 0;
    }
    
    if (_fd) {
        if (close(_fd) < 0) {
            SDL_Log("Failed to close file.");
        }
        _fd = 0;
    }
}

void MapRegion::growBackingMemory()
{
    unmapFile();
    mapFile(_backingMemorySize * 2);
}

void MapRegion::growLookupTable(size_t newCapacity)
{
    const size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
    MallocZone::Block *newBlock;
    while (!(newBlock = _zone.reallocate(lookupTableBlock(), newSize))) {
        growBackingMemory();
    }
    header()->lookupTableOffset = _zone.offsetForBlock(newBlock);
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
