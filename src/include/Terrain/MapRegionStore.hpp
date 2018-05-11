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
#include "Terrain/MapRegion.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Grid/SparseGrid.hpp"
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

// Stores/Loads voxel chunks on the file system.
class MapRegionStore
{
public:
    ~MapRegionStore() = default;
    
    // Constructor.
    // mapDirectory -- Directory where map region files should be stored.
    // boundingBox -- The bounding box is the bounds of the world and should
    //                match the voxel generator's bounds.
    // gridResolution --  The number of voxels in a chunk.
    MapRegionStore(boost::filesystem::path mapDirectory,
                   const AABB &boundingBox,
                   const glm::ivec3 &gridResolution);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<Array3D<Voxel>> load(const AABB &chunkBBox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(const AABB &boundingBox, Morton3 key, const Array3D<Voxel> &voxels);
    
private:
    boost::filesystem::path _mapDirectory;
    SparseGrid<std::shared_ptr<MapRegion>> _regions;
    
    std::shared_ptr<MapRegion> get(const glm::vec3 &p);
};

#endif /* MapRegionStore_hpp */
