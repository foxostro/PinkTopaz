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
 : TerrainOperation({location, glm::vec3(0.1f)}), // The operation affects a single voxel at the specified point in space.
   _location(location),
   _newValue(newValue)
{}

void TerrainOperationEditPoint::perform(VoxelDataChunk &chunk)
{
    chunk.set(_location, _newValue);
}
