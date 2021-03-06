//
//  BlockDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#include "BlockDataStore/BlockDataStore.hpp"

BlockDataStore::BlockDataStore(std::shared_ptr<spdlog::logger> log,
                               const boost::filesystem::path &regionFileName,
                               uint32_t magic,
                               uint32_t version)
 : _zone(log,
         regionFileName,
         InitialBackingBufferSize,
         magic,
         version)
{}

void BlockDataStore::store(Key key, const std::vector<uint8_t> &bytes)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    const size_t size = bytes.size();
    assert(size > 0);
    BoxedMallocZone::BoxedBlock block = getBlockAndResize(key, size);
    memcpy(block->data, &bytes[0], size);
    memset(block->data + size, 0, block->size - size);
}

void BlockDataStore::remove(Key key)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    _zone.deallocate(getBlock(key));
    
    // If no item has yet been stored then we won't have a lookup table.
    // In this case, do nothing.
    if (!lookupTableBlock()) {
        return;
    }
    
    // Remove the key from the lookup table.
    BlockDataStore::LookupTable &table = lookup();
    
    // First, find the entry that needs to be removed.
    ssize_t indexToRemove = -1;
    for (ssize_t i = 0, n = table.numberOfEntries; i < n; ++i) {
        auto &entry = lookup().entries[i];
        if (key == entry.key) {
            indexToRemove = i;
            break;
        }
    }
    
    // If we failed to find the item then there's nothing to do.
    if (indexToRemove < 0) {
        return;
    }
    
    // Copy subsequent entries back one slot to overwrite it.
    for (ssize_t i = indexToRemove, n = table.numberOfEntries-1; i < n; ++i) {
        auto &dst = lookup().entries[i+0];
        const auto &src = lookup().entries[i+1];
        dst = src;
    }
    
    // Decrease the number of entries.
    table.numberOfEntries--;
    
    // Overwrite the now-invalid final entry so it cannot be accidentally used.
    auto &entry = lookup().entries[table.numberOfEntries];
    entry.key=0xdeadbeef;
    entry.offset=0xdeadbeef;
}

boost::optional<std::vector<uint8_t>> BlockDataStore::load(Key key) const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    
    if (!loadOffsetForKey(key)) {
        return boost::none;
    }
    
    BoxedMallocZone::ConstBoxedBlock block = getBlock(key);
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
    auto maybeOffset = loadOffsetForKey(key);
    if (maybeOffset) {
        offset = *maybeOffset;
    }
    return _zone.blockPointerForOffset(offset);
}

BoxedMallocZone::ConstBoxedBlock BlockDataStore::getBlock(Key key) const
{
    auto offset = BoxedMallocZone::NullOffset;
    auto maybeOffset = loadOffsetForKey(key);
    if (maybeOffset) {
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
    auto header = _zone.header();
    assert(header);
    auto offset = header->lookupTableOffset;
    assert(offset != 0);
    return _zone.blockPointerForOffset(offset);
}

BoxedMallocZone::ConstBoxedBlock BlockDataStore::lookupTableBlock() const
{
    auto header = _zone.header();
    assert(header);
    auto offset = header->lookupTableOffset;
    assert(offset != 0);
    return _zone.blockPointerForOffset(offset);
}

BlockDataStore::LookupTable& BlockDataStore::lookup()
{
    return *((LookupTable *)lookupTableBlock()->data);
}

const BlockDataStore::LookupTable& BlockDataStore::lookup() const
{
    return *((const LookupTable *)lookupTableBlock()->data);
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
        uint32_t newCapacity = lookup().capacity * 2;
        growLookupTable(newCapacity);
        lookup().capacity = newCapacity;
    }
    
    // Add a new entry at the end of the table.
    lookup().entries[lookup().numberOfEntries++] = {key, offset};
}

boost::optional<uint32_t> BlockDataStore::loadOffsetForKey(Key key) const
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
