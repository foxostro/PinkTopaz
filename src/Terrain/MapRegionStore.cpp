//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"

MapRegionStore::MapRegionStore(const AABB &bbox, unsigned mapRegionSize)
 : _regions(bbox, glm::ivec3(bbox.extent*2.f) / (int)mapRegionSize)
{
    _regions.setCountLimit(128); // TODO: Look up the max file descriptors on this system.
}

boost::optional<Array3D<Voxel>>
MapRegionStore::load(const AABB &boundingBox, Morton3 key)
{
    return get(boundingBox.center)->load(boundingBox, key);
}

void MapRegionStore::store(const AABB &boundingBox,
                           Morton3 key,
                           const Array3D<Voxel> &voxels)
{
    // TODO: MapRegionStore::store() can be async.
    return get(boundingBox.center)->store(key, voxels);
}

std::shared_ptr<MapRegion> MapRegionStore::get(const glm::vec3 &p)
{
    return _regions.get(_regions.indexAtPoint(p), [=]{
        return std::make_shared<MapRegion>();
    });
}
