//
//  VoxelDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef VoxelDataStore_hpp
#define VoxelDataStore_hpp

#include "VoxelDataGenerator.hpp"
#include "ConcurrentGridMutable.hpp"

// A block of voxels in space with locking and expectation of concurrent access.
class VoxelDataStore : public ConcurrentGridMutable<Voxel>
{
public:
    VoxelDataStore(const std::shared_ptr<VoxelDataGenerator> &generator,
                   unsigned chunkSize);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, const Reader &fn) const override;
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region. It is the
    // responsibility of the caller to provide a closure which will update the
    // change log accordingly.
    // region -- The region we will be writing to.
    // fn -- Closure which will be doing the writing.
    void writerTransaction(const AABB &region, const Writer &fn) override;
};

#endif /* VoxelDataStore_hpp */
