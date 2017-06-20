//
//  VoxelDataStore.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef VoxelDataStore_hpp
#define VoxelDataStore_hpp

#include "VoxelData.hpp"
#include "VoxelDataGenerator.hpp"
#include "ConcurrentGridMutable.hpp"

// A block of voxels in space with locking and expectation of concurrent access.
// AFOX_TODO: The VoxelDataStore class has become anemic and should be removed.
class VoxelDataStore : public ConcurrentGridMutable<Voxel>
{
public:
    VoxelDataStore(const std::shared_ptr<VoxelDataGenerator> &generator,
                   unsigned chunkSize)
     : ConcurrentGridMutable<Voxel>(std::make_unique<VoxelData>(generator, chunkSize),
                                    chunkSize)
    {}
};

#endif /* VoxelDataStore_hpp */
