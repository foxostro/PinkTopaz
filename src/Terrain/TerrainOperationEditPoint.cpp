//
//  TerrainOperationEditPoint.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#include "Terrain/TerrainOperationEditPoint.hpp"
#include "SDL.h" // for SDL_Log()

TerrainOperationEditPoint::TerrainOperationEditPoint(glm::vec3 location, Voxel newValue)
 : _location(location), _newValue(newValue)
{
    // The affected region must touch the neighboring cells too as their mesh
    // representations may be affected by the change.
    AABB region;
    region.center = _location;
    region.extent = glm::vec3(2.f);
    setAffectedRegion(region);
}

void TerrainOperationEditPoint::perform(Array3D<Voxel> &voxelData)
{
    Voxel &voxel = voxelData.mutableReference(_location);
    voxel = _newValue;
}
