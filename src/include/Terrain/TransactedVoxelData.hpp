//
//  TransactedVoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#ifndef ConcurrentVoxelData_hpp
#define ConcurrentVoxelData_hpp

#include <boost/signals2.hpp>
#include <functional>
#include <memory>

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "Terrain/Voxel.hpp"
#include "Terrain/TerrainOperation.hpp"
#include "Terrain/SunlightData.hpp"

// A block of voxels in space. Concurrent edits are protected by a lock.
class TransactedVoxelData : public GridIndexer
{
public:
    // Default Destructor.
    virtual ~TransactedVoxelData() = default;
    
    // No default constructor.
    TransactedVoxelData() = delete;
    
    // Constructor.
    // source -- The source provides initial voxel data.
    TransactedVoxelData(std::unique_ptr<SunlightData> &&source);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn);
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(const std::shared_ptr<TerrainOperation> &operation);
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet);
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    AABB getAccessRegionForOperation(const std::shared_ptr<TerrainOperation> &operation);
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const AABB &affectedRegion)> onWriterTransaction;
    
private:
    mutable RegionMutualExclusionArbitrator _lockArbitrator;
    std::unique_ptr<SunlightData> _source;
};

#endif /* ConcurrentVoxelData_hpp */
