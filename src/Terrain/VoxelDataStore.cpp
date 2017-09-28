//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"

VoxelDataStore::VoxelDataStore(std::unique_ptr<VoxelData> &&voxelData, unsigned chunkSize)
 : GridIndexer(voxelData->boundingBox(), voxelData->gridResolution()),
   _arrayLocks(voxelData->boundingBox(), voxelData->gridResolution() / (int)chunkSize),
   _array(std::move(voxelData))
{}

void VoxelDataStore::readerTransaction(const AABB &region, const Reader &fn) const
{
    // TODO: The call to copy() will serially fetch the underling voxels if they
    // were not present. This will lead to other reader transactions being
    // blocked waiting for those voxels even if they might be able to make
    // progress if one or two of those voxel chunks became ready.
    // Instead, we should fire off a group of asynchronous tasks right here
    // where each task will take the lock on a single voxel chunk and fetch that
    // chunk. Once all tasks in the group has completed then we grab the entire
    // lockset and proceed with the copy as before.
    
    LockSet locks(locksForRegion(region));
    auto rawPointer = (VoxelData *)_array.get();
    assert(rawPointer != nullptr);
    const Array3D<Voxel> data = rawPointer->copy(region);
    fn(data);
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    ChangeLog changeLog;
    {
        LockSet locks(locksForRegion(region));
        VoxelData &voxels = *_array;
        changeLog = fn(voxels);
    }
    onWriterTransaction(changeLog);
}
