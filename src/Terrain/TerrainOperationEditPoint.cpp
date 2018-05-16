//
//  TerrainOperationEditPoint.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#include "Terrain/TerrainOperationEditPoint.hpp"

TerrainOperationEditPoint::TerrainOperationEditPoint(glm::vec3 location,
                                                     Voxel newValue)
 : TerrainOperation({location, glm::vec3(0.0f)}), // The operation affects a single voxel at the specified point in space.
   _location(location),
   _newValue(newValue)
{}

void TerrainOperationEditPoint::perform(Array3D<Voxel> &voxelData)
{
    Voxel &voxel = voxelData.mutableReference(_location);
    voxel = _newValue;
}
