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
#include "Malloc/ManagedMallocZone.hpp"
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
    
    std::mutex _mutex;
    VoxelDataSerializer _serializer;
    ManagedMallocZone _zone;
    
    // A call to growLookupTable() may invalidate all Block* from the zone.
    void growLookupTable(size_t newCapacity);
    
    BoxedMallocZone::BoxedBlock getBlock(Morton3 key);
    BoxedMallocZone::BoxedBlock getBlockAndResize(Morton3 key, size_t size);
    
    void stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes);
    boost::optional<std::vector<uint8_t>> getChunkBytesFromStash(Morton3 key);
    
    BoxedMallocZone::BoxedBlock lookupTableBlock();
    LookupTable& lookup();
    
    // A call to storeOffsetForKey() may invalidate all Block* from the zone.
    // This is due to having to potentially grow the lookup table and the zone.
    void storeOffsetForKey(Morton3 key, uint32_t offset);
    
    boost::optional<uint32_t> loadOffsetForKey(Morton3 key);
};

#endif /* MapRegion_hpp */
