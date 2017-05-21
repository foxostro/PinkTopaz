//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const AABB &box, const glm::ivec3 &res)
 : _chunk(box, res)
{}

Voxel VoxelData::get(const glm::vec3 &p) const
{
    return _chunk.get(p);
}

Voxel& VoxelData::getm(const glm::vec3 &p)
{
    return _chunk.getm(p);
}

Voxel VoxelData::get(const glm::vec3 &p, const Voxel &defaultValue) const
{
    return _chunk.get(p, defaultValue);
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    return _chunk.set(p, object);
}

glm::vec3 VoxelData::getCellDimensions() const
{
    return _chunk.getCellDimensions();
}

AABB VoxelData::getBoundingBox() const
{
    return _chunk.getBoundingBox();
}

glm::ivec3 VoxelData::getResolution() const
{
    return _chunk.getResolution();
}
