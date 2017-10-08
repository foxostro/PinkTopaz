//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"

MapRegionStore::MapRegionStore(const AABB &bbox, unsigned mapRegionSize)
 : GridIndexer(bbox, glm::ivec3(bbox.extent*2.f) / (int)mapRegionSize)
{}

boost::optional<Array3D<Voxel>>
MapRegionStore::load(const AABB &boundingBox, Morton3 key)
{
    auto mutex = _lockArbitrator.writerMutex(boundingBox);
    std::lock_guard<decltype(mutex)> lock(mutex);
    return _mapRegion.load(boundingBox, key);
}

void MapRegionStore::store(const AABB &boundingBox,
                           Morton3 key,
                           const Array3D<Voxel> &voxels)
{
    auto mutex = _lockArbitrator.writerMutex(boundingBox);
    std::lock_guard<decltype(mutex)> lock(mutex);
    _mapRegion.store(key, voxels);
}
