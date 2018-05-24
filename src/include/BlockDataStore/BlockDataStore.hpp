//
//  BlockDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#ifndef BlockDataStore_hpp
#define BlockDataStore_hpp

#include "BlockDataStore/ManagedMallocZone.hpp"
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <shared_mutex>

// Stores/Loads blocks of unstructured data on the file system.
class BlockDataStore
{
public:
    using Key = uint64_t;
    
    BlockDataStore(std::shared_ptr<spdlog::logger> log,
                   const boost::filesystem::path &regionFileName,
                   uint32_t magic,
                   uint32_t version);
    
    // Loads a block of data from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<std::vector<uint8_t>> load(Key key) const;
    
    // Stores a block of data to file.
    void store(Key key, const std::vector<uint8_t> &data);
    
    // Invalidates the block of data, removing it from file.
    void invalidate(Key key);
    
private:
    static constexpr size_t InitialBackingBufferSize = 128;
    static constexpr size_t InitialLookTableCapacity = 32;
    
    struct LookUpTableEntry
    {
        Key key;
        BoxedMallocZone::Offset offset;
    };
    
    struct LookupTable
    {
        uint32_t capacity;
        uint32_t numberOfEntries;
        LookUpTableEntry entries[0];
    };
    
    mutable std::shared_mutex _mutex;
    ManagedMallocZone _zone;
    
    // A call to growLookupTable() may invalidate all Block* from the zone.
    void growLookupTable(size_t newCapacity);
    
    BoxedMallocZone::BoxedBlock getBlock(Key key);
    BoxedMallocZone::ConstBoxedBlock getBlock(Key key) const;
    
    BoxedMallocZone::BoxedBlock getBlockAndResize(Key key, size_t size);
    
    BoxedMallocZone::BoxedBlock lookupTableBlock();
    BoxedMallocZone::ConstBoxedBlock lookupTableBlock() const;
    
    LookupTable& lookup();
    const LookupTable& lookup() const;
    
    void storeOffsetForKey(Key key, uint32_t offset);
    boost::optional<uint32_t> loadOffsetForKey(Key key) const;
};

#endif /* BlockDataStore_hpp */
