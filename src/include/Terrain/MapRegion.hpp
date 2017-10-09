//
//  MapRegion.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#ifndef MapRegion_hpp
#define MapRegion_hpp

#include "Terrain/Voxel.hpp"
#include "Terrain/VoxelDataSerializer.hpp"
#include "Grid/Array3D.hpp"
#include "Malloc/MallocZone.hpp"
#include "MemoryMappedFile.hpp"
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

// Stores/Loads voxel chunks on the file system.
class MapRegion
{
public:
    MapRegion(const boost::filesystem::path &regionFileName);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<Array3D<Voxel>> load(const AABB &bbox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(Morton3 key, const Array3D<Voxel> &voxels);
    
private:
    static constexpr size_t InitialBackingBufferSize = 1024 * 512;
    static constexpr size_t InitialLookTableCapacity = 256;
    static constexpr uint32_t MAP_REGION_MAGIC = 'rpam';
    
    struct LookUpTableEntry
    {
        uint64_t key;
        uint32_t offset;
    };
    
    struct LookupTable
    {
        uint32_t capacity;
        uint32_t numberOfEntries;
        LookUpTableEntry entries[0];
    };
    
    struct Header
    {
        uint32_t magic;
        uint32_t lookupTableOffset;
        uint32_t zoneSize;
        uint8_t zoneData[0];
    };
    
    std::mutex _mutex;
    MemoryMappedFile _file;
    VoxelDataSerializer _serializer;
    MallocZone _zone;
    
    inline Header* header()
    {
        Header *header = (Header *)_file.mapping();
        assert(header);
        return header;
    }
    
    void mapFile(size_t minimumFileSize);
    void unmapFile();
    
    // A call to growBackingMemory() may invalidate all Block* from the zone.
    void growBackingMemory();
    
    // A call to growLookupTable() may invalidate all Block* from the zone.
    void growLookupTable(size_t newCapacity);
    
    MallocZone::Block* getBlock(Morton3 key);
    MallocZone::Block* getBlockAndResize(Morton3 key, size_t size);
    
    void stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes);
    boost::optional<std::vector<uint8_t>> getChunkBytesFromStash(Morton3 key);
    
    inline MallocZone::Block* lookupTableBlock()
    {
        if (header()->lookupTableOffset == 0) {
            return nullptr;
        } else {
            return _zone.blockForOffset(header()->lookupTableOffset);
        }
    }
    
    inline LookupTable& lookup()
    {
        MallocZone::Block *block = lookupTableBlock();
        assert(block);
        return *((LookupTable *)block->data);
    }
    
    // A call to storeOffsetForKey() may invalidate all Block* from the zone.
    // This is due to having to potentially grow the lookup table and the zone.
    void storeOffsetForKey(Morton3 key, uint32_t offset);
    
    boost::optional<uint32_t> loadOffsetForKey(Morton3 key);
};

#endif /* MapRegion_hpp */
