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
 : _location(location), _newValue(newValue)
{
    // The operation affects a single voxel at the specified point in space.
    setVoxelWriteRegion({_location, glm::vec3(0.0f)});
    
    // The mesh effect region must touch the neighboring cells too as their mesh
    // representations may be affected by the change.
    setMeshEffectRegion({_location, glm::vec3(2.f)});
}

void TerrainOperationEditPoint::perform(Array3D<Voxel> &voxelData)
{
    Voxel &voxel = voxelData.mutableReference(_location);
    voxel = _newValue;
}
