//
//  TerrainOperationEditPoint.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#include "Terrain/TerrainOperationEditPoint.hpp"

TerrainOperationEditPoint::TerrainOperationEditPoint(glm::vec3 location, Voxel newValue)
 : _location(location), _newValue(newValue)
{
    AABB region;
    region.center = _location;
    region.extent = glm::vec3(1.f);
    setAffectedRegion(region);
}

void TerrainOperationEditPoint::perform(Array3D<Voxel> &voxelData)
{
    Voxel &voxel = voxelData.mutableReference(_location);
    voxel = _newValue;
}
