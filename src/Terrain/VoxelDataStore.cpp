//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"
#include <mutex> // for std::unique_lock

VoxelDataStore::VoxelDataStore(const AABB &box, const glm::ivec3 &resolution)
 : _data(box, resolution),
   _chunkLocks(box, _data.gridResolution())
{
    for (auto &lock : _chunkLocks) {
        lock = std::make_shared<std::shared_mutex>();
    }
}

void VoxelDataStore::underLock(const AABB &region,
                               bool shared,
                               const std::function<void()> &fn) const
{
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    
    _chunkLocks.forEachCell(region, [&](const AABB &cell){
        std::shared_ptr<std::shared_mutex> lock = _chunkLocks.get(cell.center);
        locks.push_back(lock);
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
        const std::shared_ptr<std::shared_mutex> &lock = *iter;
        if (shared) {
            lock->unlock_shared();
        } else {
            lock->unlock();
        }
    }
}

void VoxelDataStore::readerTransaction(const AABB &region, const std::function<void(const Array3D<Voxel> &voxels)> &fn) const
{
    underLock(region, true, [&]{
        Array3D<Voxel> aCopy = _data.copy(region);
        fn(aCopy);
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
