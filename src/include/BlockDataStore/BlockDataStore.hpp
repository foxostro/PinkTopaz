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
#include <mutex>

// Stores/Loads blocks of unstructured data on the file system.
class BlockDataStore
{
public:
    using Key = uint64_t;
    
    BlockDataStore(const boost::filesystem::path &regionFileName);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<std::vector<uint8_t>> load(Key key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(Key key, const std::vector<uint8_t> &data);
    
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
    
    std::mutex _mutex;
    ManagedMallocZone _zone;
    
    // A call to growLookupTable() may invalidate all Block* from the zone.
    void growLookupTable(size_t newCapacity);
    
    BoxedMallocZone::BoxedBlock getBlock(Key key);
    BoxedMallocZone::BoxedBlock getBlockAndResize(Key key, size_t size);
    
    BoxedMallocZone::BoxedBlock lookupTableBlock();
    LookupTable& lookup();
    
    void storeOffsetForKey(Key key, uint32_t offset);
    boost::optional<uint32_t> loadOffsetForKey(Key key);
};

#endif /* BlockDataStore_hpp */
