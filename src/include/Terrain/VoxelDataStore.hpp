//
//  VoxelDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef VoxelDataStore_hpp
#define VoxelDataStore_hpp

#include <boost/signals2.hpp>
#include <functional>
#include <vector>
#include <mutex>

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/RegionMutualExclusionArbitrator.hpp"
#include "ChangeLog.hpp"
#include "VoxelData.hpp"

// Wraps a VoxelData object. Transactions on VoxelDataStore protect this voxel
// data to allow concurrent readers and writers.
class VoxelDataStore : public GridIndexer
{
public:
    using Reader = std::function<void(const Array3D<Voxel> &data)>;
    using Writer = std::function<ChangeLog(VoxelData &data)>;
    
    // Destructor is just the default.
    virtual ~VoxelDataStore() = default;
    
    // No default constructor.
    VoxelDataStore() = delete;
    
    // Constructor. Accepts a VoxelData object which contains the actual voxels
    // Transactions on VoxelDataStore protect this voxel data during concurrent
    // access.
    VoxelDataStore(std::unique_ptr<VoxelData> &&voxelData, unsigned chunkSize);
    
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
    
protected:
    mutable RegionMutualExclusionArbitrator _lockArbitrator;
    std::unique_ptr<VoxelData> _array;
};

#endif /* VoxelDataStore_hpp */
