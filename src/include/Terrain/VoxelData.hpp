//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Grid/SparseGrid.hpp"
#include "Terrain/VoxelDataSource.hpp"
#include "Terrain/PersistentVoxelChunks.hpp"

#include <spdlog/spdlog.h>

// A block of voxels in space.
class VoxelData : public VoxelDataSource
{
public:
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source -- The source provides initial voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // mapRegionStore -- The map file in which to persist chunks.
    VoxelData(std::shared_ptr<spdlog::logger> log,
              std::unique_ptr<VoxelDataSource> &&source,
              unsigned chunkSize,
              std::unique_ptr<MapRegionStore> &&mapRegionStore);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn) override;
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(const std::shared_ptr<TerrainOperation> &operation) override;
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet) override;
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    AABB getAccessRegionForOperation(const std::shared_ptr<TerrainOperation> &operation) override;
    
private:
    std::shared_ptr<spdlog::logger> _log;
    std::unique_ptr<VoxelDataSource> _source;
    PersistentVoxelChunks _chunks;
    
    std::unique_ptr<PersistentVoxelChunks::Chunk> createNewChunk(const AABB &cell);
};

#endif /* VoxelData_hpp */
