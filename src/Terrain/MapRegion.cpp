//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"

MapRegion::MapRegion(std::shared_ptr<spdlog::logger> log,
                     const boost::filesystem::path &regionFileName)
 : _dataStore(log, regionFileName, 'rpam', 0),
   _log(log)
{}

boost::optional<Array3D<Voxel>> MapRegion::load(const AABB &bbox, Morton3 key)
{
    boost::optional<std::vector<uint8_t>> maybeBytes(_dataStore.load((size_t)key));
    
    if (maybeBytes) {
        const auto &bytes = *maybeBytes;
        try {
            const auto chunk = _serializer.load(bbox, bytes);
            return boost::make_optional(chunk);
        } catch(const VoxelDataException &exception) {
            _log->error("MapRegion failed to deserialize voxels."\
                        "Treating as-if it is not cached.");
        }
    }
    
    return boost::none;
}

void MapRegion::store(Morton3 key, const Array3D<Voxel> &voxels)
{
    _dataStore.store((size_t)key, _serializer.store(voxels));
}

void MapRegion::invalidate(Morton3 key)
{
    _dataStore.invalidate((size_t)key);
}
