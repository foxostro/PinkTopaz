//
//  VoxelDataSource.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/14/18.
//
//

#ifndef VoxelDataSource_hpp
#define VoxelDataSource_hpp

#include <boost/signals2.hpp>
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
    virtual void writerTransaction(const std::shared_ptr<TerrainOperation> &operation) = 0;
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const AABB &affectedRegion)> onWriterTransaction;
};

#endif /* VoxelDataSource_hpp */
