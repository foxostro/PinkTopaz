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
#include <boost/optional.hpp>

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
    RegionMutualExclusionArbitrator _lockArbitrator;
    VoxelDataSerializer _serializer;
};

#endif /* MapRegionStore_hpp */
