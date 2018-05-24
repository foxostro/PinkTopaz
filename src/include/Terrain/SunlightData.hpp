//
//  SunlightData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#ifndef SunlightData_hpp
#define SunlightData_hpp

#include "Terrain/VoxelData.hpp"
#include "Terrain/PersistentVoxelChunks.hpp"

#include <spdlog/spdlog.h>

// Voxels with sunlight computation.
class SunlightData : public GridIndexer
{
public:
    // No default constructor.
    SunlightData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source -- The source provides initial, unlit voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // mapRegionStore -- The map file in which to persist chunks.
    SunlightData(std::shared_ptr<spdlog::logger> log,
                 std::unique_ptr<VoxelData> &&source,
                 unsigned chunkSize,
                 std::unique_ptr<MapRegionStore> &&mapRegionStore);
    
    // Loads the chunk at the specified region of space.
    // The specified region of space is not required to exactly match the
    // position and size of one of the chunks used internally by the sunlight
    // data grid. This can be an arbitrary region of space with the bounds of
    // the grid.
    // May fault in missing voxels to satisfy the request.
    Array3D<Voxel> load(const AABB &region);
    
    // Edits a single voxel.
    void editSingleVoxel(const glm::vec3 &point, const Voxel &value);
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet);
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    AABB getAccessRegionForOperation(TerrainOperation &operation);
    
private:
    std::shared_ptr<spdlog::logger> _log;
    std::unique_ptr<VoxelData> _source;
    PersistentVoxelChunks _chunks;
    
    std::unique_ptr<VoxelDataChunk> createNewChunk(const AABB &cell, Morton3 index);
    
    AABB getSunlightRegion(AABB sunlightRegion) const;
};

#endif /* SunlightData_hpp */
