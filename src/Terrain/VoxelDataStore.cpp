//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"

VoxelDataStore::VoxelDataStore(std::unique_ptr<VoxelData> &&voxelData)
 : GridIndexer(voxelData->boundingBox(), voxelData->gridResolution()),
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
    
    auto mutex = _lockArbitrator.getMutex(region);
    std::lock_guard<decltype(mutex)> lock(mutex);
    
    auto rawPointer = (VoxelData *)_array.get();
    assert(rawPointer != nullptr);
    const Array3D<Voxel> data = rawPointer->copy(region);
    fn(data);
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    ChangeLog changeLog;
    {
        auto mutex = _lockArbitrator.getMutex(region);
        std::lock_guard<decltype(mutex)> lock(mutex);
        changeLog = fn(*_array);
    }
    onWriterTransaction(changeLog);
}
