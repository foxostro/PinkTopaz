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

boost::optional<VoxelDataChunk>
MapRegionStore::load(const AABB &boundingBox, Morton3 key)
{
    return get(boundingBox.center)->load(boundingBox, key);
}

void MapRegionStore::store(const AABB &boundingBox,
                           Morton3 key,
                           const VoxelDataChunk &chunk)
{
    // TODO: MapRegionStore::store() can be async.
    get(boundingBox.center)->store(key, chunk);
}

std::shared_ptr<MapRegion> MapRegionStore::get(const glm::vec3 &p)
{
    const Morton3 index = _regions.indexAtPoint(p);
    return _regions.get(index, [=]{
        std::lock_guard<std::mutex> lock(_mutex);
        boost::filesystem::path name("MapRegion_" + std::to_string((size_t)index) + ".bin");
        boost::filesystem::path path(_mapDirectory / name);
        return std::make_shared<MapRegion>(_log, path);
    });
}

void MapRegionStore::invalidate(const glm::vec3 &point, Morton3 key)
{
    get(point)->invalidate(key);
}
