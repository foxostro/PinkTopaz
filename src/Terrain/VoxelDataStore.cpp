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

void VoxelDataStore::readerTransaction(const AABB &region, const Reader &fn) const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    fn(_data.copy(region));
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    ChangeLog changeLog;
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        changeLog = fn(_data);
    }
    voxelDataChanged(changeLog);
}

glm::vec3 VoxelDataStore::cellDimensions() const
{
    return _data.cellDimensions();
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
