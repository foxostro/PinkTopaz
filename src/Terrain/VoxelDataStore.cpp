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

void VoxelDataStore::readerTransaction(const AABB &region, const std::function<void(const Array3D<Voxel> &voxels)> &fn) const
{
    // AFOX_TODO: Can I extract the lock taking stuff to it's own method?
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    _chunkLocks.forEachCell(region, [&](const AABB &cell){
        std::shared_ptr<std::shared_mutex> lock = _chunkLocks.get(cell.center);
        locks.push_back(lock);
    });
    
    for (const std::shared_ptr<std::shared_mutex> &lock : locks) {
        lock->lock_shared();
    }
    
    Array3D<Voxel> aCopy = _data.copy(region);
    fn(aCopy); // AFOX_TODO: Can this be moved out of the locked region, or is that a data race?
    
    for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter)
    {
        const std::shared_ptr<std::shared_mutex> &lock = *iter;
        lock->unlock_shared();
    }
}

void VoxelDataStore::writerTransaction(const AABB &region, const std::function<ChangeLog(GridMutable<Voxel> &voxels)> &fn)
{
    ChangeLog changeLog;
    {
        std::vector<std::shared_ptr<std::shared_mutex>> locks;
        _chunkLocks.forEachCell(region, [&](const AABB &cell){
            const std::shared_ptr<std::shared_mutex> &lock = _chunkLocks.get(cell.center);
            locks.push_back(lock);
        });
        
        for (const std::shared_ptr<std::shared_mutex> &lock : locks) {
            lock->lock();
        }
        
        GridViewMutable<Voxel> view = _data.getView(region);
        changeLog = fn(view);
        
        for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter)
        {
            const std::shared_ptr<std::shared_mutex> &lock = *iter;
            lock->unlock();
        }
    }
    voxelDataChanged(changeLog);
}
