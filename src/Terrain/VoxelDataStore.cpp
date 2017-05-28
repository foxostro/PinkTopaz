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

void VoxelDataStore::readerTransaction(const AABB &region, const std::function<void(const GridAddressable<Voxel> &voxels)> &fn) const
{
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    _chunkLocks.forEachCell(region, [&](const AABB &cell){
        std::shared_ptr<std::shared_mutex> lock = _chunkLocks.get(cell.center);
        locks.push_back(lock);
    });
    
    for (const std::shared_ptr<std::shared_mutex> &lock : locks) {
        lock->lock_shared();
    }
    
    fn(_data.getView(region));
    
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
