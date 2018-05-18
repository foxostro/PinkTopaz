//
//  ConcurrentVoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#ifndef ConcurrentVoxelData_hpp
#define ConcurrentVoxelData_hpp

#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Terrain/VoxelDataSource.hpp"
#include <spdlog/spdlog.h>

// A block of voxels in space. Concurrent edits are protected by a lock.
class ConcurrentVoxelData : public VoxelDataSource
{
public:
    // Default Destructor.
    virtual ~ConcurrentVoxelData() = default;
    
    // No default constructor.
    ConcurrentVoxelData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source -- The source provides initial voxel data.
    ConcurrentVoxelData(std::shared_ptr<spdlog::logger> log,
                        std::unique_ptr<VoxelDataSource> &&source);
    
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
    
private:
    mutable RegionMutualExclusionArbitrator _lockArbitrator;
    std::shared_ptr<spdlog::logger> _log;
    std::unique_ptr<VoxelDataSource> _source;
};

#endif /* ConcurrentVoxelData_hpp */
