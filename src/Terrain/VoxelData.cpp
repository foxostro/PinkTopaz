//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const AABB &box, const glm::ivec3 &res)
: _voxels(res.x * res.y * res.z), _box(box), _res(res)
{}

const Voxel& VoxelData::get(const glm::vec3 &p) const
{
    return get(indexAtPoint(p));
}

void VoxelData::set(const glm::vec3 &p, const Voxel &voxel)
{
    return set(indexAtPoint(p), voxel);
}

size_t VoxelData::indexAtPoint(const glm::vec3 &point) const
{
    const glm::vec3 mins = _box.center - _box.extent;
    const glm::vec3 p = (point - mins) / (_box.extent*2.0f);
    const glm::ivec3 a(p.x * _res.x, p.y * _res.y, p.z * _res.z);
    
    // Columns in the y-axis are contiguous in memory.
    return (a.x * _res.y * _res.z) + (a.z * _res.y) + a.y;
}

const Voxel& VoxelData::get(size_t index) const
{
    return _voxels[index];
}

void VoxelData::set(size_t index, const Voxel &voxel)
{
    _voxels[index] = voxel;
}
