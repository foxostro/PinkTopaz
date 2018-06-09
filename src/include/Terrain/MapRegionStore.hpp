//
//  MapRegionStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#ifndef MapRegionStore_hpp
#define MapRegionStore_hpp

#include "Terrain/VoxelDataSerializer.hpp"
#include "Terrain/MapRegion.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Grid/ConcurrentSparseGrid.hpp"
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

// Stores/Loads voxel chunks on the file system.
class MapRegionStore
{
public:
    ~MapRegionStore() = default;
    
    // Constructor.
    // log -- Which log are we logging to?
    // mapDirectory -- Directory where map region files should be stored.
    // boundingBox -- The bounding box is the bounds of the world and should
    //                match the voxel generator's bounds.
    // gridResolution --  The number of voxels in a chunk.
    MapRegionStore(std::shared_ptr<spdlog::logger> log,
                   boost::filesystem::path mapDirectory,
                   const AABB &boundingBox,
                   const glm::ivec3 &gridResolution);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<VoxelDataChunk> load(const AABB &chunkBBox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(const AABB &boundingBox, Morton3 key, const VoxelDataChunk &chunk);
    
private:
    boost::filesystem::path _mapDirectory;
    ConcurrentSparseGrid<std::shared_ptr<MapRegion>> _regions;
    std::shared_ptr<spdlog::logger> _log;
    std::mutex _mutex;
    
    std::shared_ptr<MapRegion> get(const glm::vec3 &p);
};

#endif /* MapRegionStore_hpp */
