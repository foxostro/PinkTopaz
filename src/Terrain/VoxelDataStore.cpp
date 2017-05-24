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
 : _data(box, resolution)
{}

void VoxelDataStore::readerTransaction(const AABB &region, const std::function<void(const GridAddressable<Voxel> &voxels)> &fn) const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    const GridView<Voxel> view = _data.getView(region);
    fn(view);
}

void VoxelDataStore::writerTransaction(const AABB &region, const std::function<ChangeLog(GridMutable<Voxel> &voxels)> &fn)
{
    ChangeLog changeLog;
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        GridViewMutable<Voxel> view = _data.getView(region);
        changeLog = fn(view);
    }
    voxelDataChanged(changeLog);
}
