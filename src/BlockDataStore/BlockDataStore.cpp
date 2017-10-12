//
//  BlockDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#include "BlockDataStore/BlockDataStore.hpp"
#include "SDL.h" // for SDL_Log

BlockDataStore::BlockDataStore(const boost::filesystem::path &regionFileName)
 : _zone(regionFileName, InitialBackingBufferSize)
{}

void BlockDataStore::store(Key key, const std::vector<uint8_t> &bytes)
{
    const size_t size = bytes.size();
    assert(size > 0);
    BoxedMallocZone::BoxedBlock block = getBlockAndResize(key, size);
    memcpy(block->data, &bytes[0], size);
    memset(block->data + size, 0, block->size - size);
}

boost::optional<std::vector<uint8_t>> BlockDataStore::load(Key key)
{
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

void BlockDataStore::growLookupTable(size_t newCapacity)
{
    const size_t newSize = sizeof(LookupTable) + sizeof(LookUpTableEntry) * newCapacity;
    auto lookUpTable = lookupTableBlock();
    auto newBlock = _zone.reallocate(std::move(lookUpTable), newSize);
    assert(newBlock);
    _zone.header()->lookupTableOffset = newBlock.getOffset();
}

BoxedMallocZone::BoxedBlock BlockDataStore::getBlock(Key key)
{
    auto offset = BoxedMallocZone::NullOffset;
    if (auto maybeOffset = loadOffsetForKey(key); maybeOffset) {
        offset = *maybeOffset;
    }
    return _zone.blockPointerForOffset(offset);
}

BoxedMallocZone::BoxedBlock BlockDataStore::getBlockAndResize(Key key, size_t size)
{
    auto block = _zone.reallocate(getBlock(key), size);
    assert(block);
    storeOffsetForKey(key, block.getOffset());
    return block;
}

BoxedMallocZone::BoxedBlock BlockDataStore::lookupTableBlock()
{
    return _zone.blockPointerForOffset(_zone.header()->lookupTableOffset);
}

BlockDataStore::LookupTable& BlockDataStore::lookup()
{
    return *((LookupTable *)lookupTableBlock()->data);
}

void BlockDataStore::storeOffsetForKey(Key key, BoxedMallocZone::Offset offset)
{
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

boost::optional<uint32_t> BlockDataStore::loadOffsetForKey(Key key)
{    
    if (lookupTableBlock()) {
        for (size_t i = 0, n = lookup().numberOfEntries; i < n; ++i) {
            auto &entry = lookup().entries[i];
            if (key == entry.key) {
                return entry.offset;
            }
        }
    }
    return boost::none;
}
