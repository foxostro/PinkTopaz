//
//  VoxelDataSource.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/14/18.
//
//

#ifndef VoxelDataSource_hpp
#define VoxelDataSource_hpp

#include <functional>
#include <memory>

#include "Exception.hpp"
#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Voxel.hpp"
#include "Terrain/TerrainOperation.hpp"

// Exception thrown when a writer transaction is attempted on a read-only voxel
// data source.
class VoxelDataReadOnlyException : public Exception
{
public:
    template<typename... Args>
    VoxelDataReadOnlyException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

// A source for voxel data.
class VoxelDataSource : public GridIndexer
{
public:
    // Default Destructor.
    virtual ~VoxelDataSource() = default;
    
    // No default constructor.
    VoxelDataSource() = delete;
    
    // Constructor.
    // boundingBox -- The region for which voxels are defined.
    // gridResolution -- The resolution of the voxel grid.
    VoxelDataSource(const AABB &boundingBox,
                    const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution)
    {}
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    virtual void readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn) = 0;
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    virtual void writerTransaction(TerrainOperation &operation) = 0;
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    virtual void setWorkingSet(const AABB &workingSet) = 0;
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    virtual AABB getAccessRegionForOperation(const std::shared_ptr<TerrainOperation> &operation) = 0;
};

#endif /* VoxelDataSource_hpp */
