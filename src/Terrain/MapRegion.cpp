//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"
#include "SDL.h" // for SDL_Log

MapRegion::MapRegion(const boost::filesystem::path &regionFileName)
 : _file(regionFileName)
{
    mapFile(InitialBackingBufferSize);
}

boost::optional<Array3D<Voxel>> MapRegion::load(const AABB &bbox, Morton3 key)
{
    boost::optional<std::vector<uint8_t>> maybeBytes(getChunkBytesFromStash(key));
    
    if (maybeBytes) {
        const auto &bytes = *maybeBytes;
        try {
            const auto chunk = _serializer.load(bbox, bytes);
            return boost::make_optional(chunk);
        } catch(const VoxelDataException &exception) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "MapRegion failed to deserialize voxels."\
                         "Treating as-if it is not cached.");
        }
    }
    
    return boost::none;
}

void MapRegion::store(Morton3 key, const Array3D<Voxel> &voxels)
{
    stashChunkBytes(key, _serializer.store(voxels));
}

void MapRegion::mapFile(size_t minimumFileSize)
{
    _file.unmapFile();
    bool mustReset = _file.mapFile(minimumFileSize);
    size_t newZoneSize = _file.size() - sizeof(Header);
    
    if (mustReset) {
        header()->magic = MAP_REGION_MAGIC;
        header()->version = MAP_REGION_VERSION;
        header()->zoneSize = newZoneSize;
        header()->lookupTableOffset = BoxedMallocZone::NullOffset;
        _zone.reset(header()->zoneData, header()->zoneSize);
    } else {
        if (header()->magic != MAP_REGION_MAGIC) {
            throw Exception("Unexpected magic number in map region file: found %d but expected %d", header()->magic, MAP_REGION_MAGIC);
        }
        
        if (header()->version != MAP_REGION_VERSION) {
            throw Exception("Unexpected version number in map region file: found %d but expected %d", header()->version, MAP_REGION_VERSION);
        }
        
        _zone.grow(header()->zoneData, newZoneSize);
    }
}

void MapRegion::growBackingMemory()
{
    SDL_Log("growBackingMemory");
    mapFile(_file.size() * 2);
}

void MapRegion::growLookupTable(size_t newCapacity)
{
    SDL_Log("growLookupTable");
    
    const size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
    
    while (true) {
        auto offsetOfLookupTable = lookupTableBlock().getOffset();
        auto newBlock = _zone.reallocate(offsetOfLookupTable, newSize);
        if (!newBlock) {
            growBackingMemory();
        } else {
            header()->lookupTableOffset = newBlock.getOffset();
            break;
        }
    }
}

BoxedMallocZone::BlockPointer MapRegion::getBlock(Morton3 key)
{
    if (auto maybeOffset = loadOffsetForKey(key); maybeOffset) {
        return _zone.blockPointerForOffset(*maybeOffset);
    } else {
        return BoxedMallocZone::BlockPointer(_zone);
    }
}

BoxedMallocZone::BlockPointer MapRegion::getBlockAndResize(Morton3 key, size_t size)
{
    BoxedMallocZone::Offset offset = BoxedMallocZone::NullOffset;
    BoxedMallocZone::BlockPointer block = getBlock(key);
    
    if (block) {
        offset = block.getOffset();
    }
    
    while (true) {
        block = _zone.reallocate(block.getOffset(), size);
        
        if (block) {
            storeOffsetForKey(key, block.getOffset());
            return block;
        } else {
            growBackingMemory();
        }
    }
}

void MapRegion::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    const size_t size = bytes.size();
    assert(size > 0);
    BoxedMallocZone::BlockPointer block = getBlockAndResize(key, size);
    memcpy(block->data, &bytes[0], size);
    memset(block->data + size, 0, block->size - size);
}

boost::optional<std::vector<uint8_t>>
MapRegion::getChunkBytesFromStash(Morton3 key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (!loadOffsetForKey(key)) {
        return boost::none;
    }
    
    BoxedMallocZone::BlockPointer block = getBlock(key);
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
