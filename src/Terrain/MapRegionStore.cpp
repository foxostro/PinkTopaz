//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"

MapRegionStore::MapRegionStore(const AABB &boundingBox, unsigned mapRegionSize)
: GridIndexer(boundingBox, glm::ivec3(boundingBox.extent*2.f) / (int)mapRegionSize)
{}

boost::optional<Array3D<Voxel>> MapRegionStore::load(Morton3 key)
{
    // stub
    return boost::none;
}

void MapRegionStore::store(Morton3 key, const Array3D<Voxel> &voxels)
{
    // stub
}
