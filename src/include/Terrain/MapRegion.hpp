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
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Grid/Array3D.hpp"
#include "Malloc/MallocZone.hpp"
#include <boost/optional.hpp>
#include <map>

// Stores/Loads voxel chunks on the file system.
class MapRegion
{
public:
    ~MapRegion();
    MapRegion();
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<Array3D<Voxel>> load(const AABB &bbox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(Morton3 key, const Array3D<Voxel> &voxels);
    
private:
    static constexpr size_t InitialBackingBufferSize = 256;
    
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
    
    size_t _zoneBackingMemorySize;
    uint8_t *_zoneBackingMemory;
    MallocZone _zone;
    LookupTable *_lookup;
    
    MallocZone::Block* getBlock(Morton3 key);
    MallocZone::Block* getBlockAndResize(Morton3 key, size_t size);
    
    void stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes);
    boost::optional<std::vector<uint8_t>> getChunkBytesFromStash(Morton3 key);
    
    void storeOffsetForKey(Morton3 key, uint32_t offset);
    boost::optional<uint32_t> loadOffsetForKey(Morton3 key);
};

#endif /* MapRegion_hpp */
