//
//  TransactedVoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/TransactedVoxelData.hpp"

TransactedVoxelData::TransactedVoxelData(std::unique_ptr<VoxelData> &&voxelData)
 : GridIndexer(voxelData->boundingBox(), voxelData->gridResolution()),
   _array(std::move(voxelData))
{}

void TransactedVoxelData::readerTransaction(const AABB &region, const Reader &fn) const
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
    const Array3D<Voxel> data = _array->load(region);
    fn(data);
}

void TransactedVoxelData::writerTransaction(const AABB &region, const Writer &fn)
{
    ChangeLog changeLog;
    {
        auto mutex = _lockArbitrator.getMutex(region);
        std::lock_guard<decltype(mutex)> lock(mutex);
        Array3D<Voxel> data = _array->load(region);
        changeLog = fn(data);
        _array->store(data);
    }
    onWriterTransaction(changeLog);
}
