//
//  MapRegionStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#ifndef MapRegionStore_hpp
#define MapRegionStore_hpp

#include "Terrain/Voxel.hpp"
#include "Terrain/VoxelDataSerializer.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Grid/Array3D.hpp"
#include "Malloc/MallocZone.hpp"
#include <boost/optional.hpp>
#include <map>

// Stores/Loads voxel chunks on the file system.
class MapRegionStore : GridIndexer
{
public:
    ~MapRegionStore() = default;
    
    // Constructor.
    // boundingBox -- The bounding box is the bounds of the world and should
    //                match the voxel generator's bounds.
    // mapRegionSize -- The world is divided into a grid of map regions, each of
    //                  this size.
    MapRegionStore(const AABB &boundingBox, unsigned mapRegionSize);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<Array3D<Voxel>> load(const AABB &chunkBBox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(const AABB &boundingBox, Morton3 key, const Array3D<Voxel> &voxels);
    
private:
    std::mutex _mutex;
    VoxelDataSerializer _serializer;
    
    size_t _zoneBackingMemorySize;
    uint8_t *_zoneBackingMemory;
    MallocZone _zone;
    std::map<Morton3, unsigned> _lookup;
    
    bool hasBlock(Morton3 key);
    MallocZone::Block* getBlock(Morton3 key);
    MallocZone::Block* getBlockAndResize(Morton3 key, size_t size);
    
    void stashChunkBytes(Morton3 key, const std::vector<uint8_t> &bytes);
    boost::optional<std::vector<uint8_t>> getChunkBytesFromStash(Morton3 key);
};

#endif /* MapRegionStore_hpp */
