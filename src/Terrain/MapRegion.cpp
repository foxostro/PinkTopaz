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
 : _zone(regionFileName, InitialBackingBufferSize)
{}

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

void MapRegion::growLookupTable(size_t newCapacity)
{
    const size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
    auto lookUpTable = lookupTableBlock();
    auto newBlock = _zone.reallocate(std::move(lookUpTable), newSize);
    assert(newBlock);
    _zone.header()->lookupTableOffset = newBlock.getOffset();
}

BoxedMallocZone::BoxedBlock MapRegion::getBlock(Morton3 key)
{
    auto offset = BoxedMallocZone::NullOffset;
    if (auto maybeOffset = loadOffsetForKey(key); maybeOffset) {
        offset = *maybeOffset;
    }
    return _zone.blockPointerForOffset(offset);
}

BoxedMallocZone::BoxedBlock MapRegion::getBlockAndResize(Morton3 key, size_t size)
{
    auto block = _zone.reallocate(getBlock(key), size);
    assert(block);
    storeOffsetForKey(key, block.getOffset());
    return block;
}

void MapRegion::stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    const size_t size = bytes.size();
    assert(size > 0);
    BoxedMallocZone::BoxedBlock block = getBlockAndResize(key, size);
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
    
    BoxedMallocZone::BoxedBlock block = getBlock(key);
    const size_t size = block->size;
    
    std::vector<uint8_t> bytes(size);
    assert(bytes.size() >= size);
    memcpy(&bytes[0], block->data, size);
    
    return boost::make_optional(bytes);
}

BoxedMallocZone::BoxedBlock MapRegion::lookupTableBlock()
{
    return _zone.blockPointerForOffset(_zone.header()->lookupTableOffset);
}

MapRegion::LookupTable& MapRegion::lookup()
{
    return *((LookupTable *)lookupTableBlock()->data);
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
