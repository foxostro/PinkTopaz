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

VoxelDataStore::VoxelDataStore()
 : _data(_generator)
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

AABB VoxelDataStore::boundingBox() const
{
    AABB box = _generator.boundingBox().inset(glm::vec3(16.f, 16.f, 16.f));
    return box;
}

glm::ivec3 VoxelDataStore::gridResolution() const
{
    glm::ivec3 res = _generator.gridResolution() - glm::ivec3(32, 32, 32);
    return res;
}
