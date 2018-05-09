//
//  TransactedVoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef TransactedVoxelData_hpp
#define TransactedVoxelData_hpp

#include <boost/signals2.hpp>
#include <functional>

#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "VoxelData.hpp"

// Provides an interface for concurrent manipulation of a VoxelData object.
class TransactedVoxelData : public GridIndexer
{
public:
    using Reader = std::function<void(const Array3D<Voxel> &data)>;
    using Writer = std::function<ChangeLog(Array3D<Voxel> &data)>;
    
    // Default destructor.
    ~TransactedVoxelData() = default;
    
    // No default constructor.
    TransactedVoxelData() = delete;
    
    // Constructor. Accepts a VoxelData object which contains the actual voxels.
    TransactedVoxelData(std::unique_ptr<VoxelData> &&voxelData);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, const Reader &fn) const;
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    void writerTransaction(const AABB &region, const Writer &fn);
    
    // This signal fires when a "writer" transaction finishes. This provides the
    // opportunity to respond to changes to data. For example, by rebuilding
    // meshes associated with underlying voxel data.
    boost::signals2::signal<void (const ChangeLog &changeLog)> onWriterTransaction;
    
    // We may evict chunks to keep the total chunk count under this limit.
    // Pass in at least the expected number of chunks in the working set.
    void setChunkCountLimit(unsigned chunkCountLimit)
    {
        _array->setChunkCountLimit(chunkCountLimit);
    }
    
protected:
    mutable RegionMutualExclusionArbitrator _lockArbitrator;
    std::unique_ptr<VoxelData> _array;
};

#endif /* TransactedVoxelData_hpp */
