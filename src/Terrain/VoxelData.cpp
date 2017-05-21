//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const AABB &box, const glm::ivec3 &res)
 : _box(box),
   _res(res),
   _cellDim(box.extent.x * 2.0f / res.x,
            box.extent.y * 2.0f / res.y,
            box.extent.z * 2.0f / res.z)
{}

Voxel VoxelData::get(const glm::vec3 &p) const
{
    return _chunk->get(p);
}

Voxel& VoxelData::getm(const glm::vec3 &p)
{
    return _chunk->getm(p);
}

Voxel VoxelData::get(const glm::vec3 &p, const Voxel &defaultValue) const
{
    if (!_chunk) {
        return defaultValue;
    } else {
        return _chunk->get(p, defaultValue);
    }
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    if (!_chunk) {
        return _chunk.emplace(_box, _res);
    }
    
    _chunk->set(p, object);
}

glm::vec3 VoxelData::getCellDimensions() const
{
    return _cellDim;
}

AABB VoxelData::getBoundingBox() const
{
    return _box;
}

glm::ivec3 VoxelData::getResolution() const
{
    return _res;
}
