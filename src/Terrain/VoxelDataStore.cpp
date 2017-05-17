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

void VoxelDataStore::readerTransaction(const std::function<void(const VoxelData &voxels)> &fn) const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    fn(_data);
}

void VoxelDataStore::writerTransaction(const std::function<void(VoxelData &voxels)> &fn)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    fn(_data);
}
