//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"

MapRegionStore::MapRegionStore(std::shared_ptr<spdlog::logger> log,
                               boost::filesystem::path mapDirectory,
                               const AABB &bbox,
                               const glm::ivec3 &res)
 : _mapDirectory(mapDirectory),
   _regions(bbox, res),
   _log(log)
{}

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
    const Morton3 index = _regions.indexAtPoint(p);
    return _regions.get(index, [=]{
        boost::filesystem::path name("MapRegion_" + std::to_string((size_t)index) + ".bin");
        boost::filesystem::path path(_mapDirectory / name);
        return std::make_shared<MapRegion>(_log, path);
    });
}
