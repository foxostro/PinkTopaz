//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"
#include "Profiler.hpp"
#include <mutex> // for std::unique_lock

VoxelDataStore::VoxelDataStore(const AABB &box, const glm::ivec3 &resolution)
 : _data(box, resolution),
   _chunkLocks(box, _data.gridResolution())
{
    PROFILER("VoxelDataStore::VoxelDataStore");
}

void VoxelDataStore::underLock(const AABB &region,
                               bool shared,
                               const std::function<void()> &fn) const
{
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    
    _chunkLocks.forEachCell(region, [&](const AABB &cell){
        std::lock_guard<std::mutex> lock(_lockChunkLocks);
        std::shared_ptr<std::shared_mutex> &cellLock = _chunkLocks.mutableReference(cell.center);
        if (!cellLock) {
            cellLock = std::make_shared<std::shared_mutex>();
        }
        locks.push_back(cellLock);
    });
    
    for (const std::shared_ptr<std::shared_mutex> &lock : locks) {
        if (shared) {
            lock->lock_shared();
        } else {
            lock->lock();
        }
    }
    
    fn();
    
    for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter)
    {
        const std::shared_ptr<std::shared_mutex> &cellLock = *iter;
        if (shared) {
            cellLock->unlock_shared();
        } else {
            cellLock->unlock();
        }
    }
}

void VoxelDataStore::readerTransaction(const AABB &region, const std::function<void(const Array3D<Voxel> &voxels)> &fn) const
{
    underLock(region, true, [&]{
        fn(_data.copy(region));
    });
}

void VoxelDataStore::writerTransaction(const AABB &region, const std::function<ChangeLog(GridMutable<Voxel> &voxels)> &fn)
{
    ChangeLog changeLog;
    underLock(region, false, [&]{
        GridViewMutable<Voxel> view = _data.getView(region);
        changeLog = fn(view);
    });
    voxelDataChanged(changeLog);
}
