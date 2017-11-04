//
//  MapRegion.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#include "Terrain/MapRegion.hpp"
#include "SDL.h" // for SDL_Log

MapRegion::MapRegion(const std::string &regionFileName)
 : _dataStore(regionFileName, 'rpam', 0)
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
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "MapRegion failed to deserialize voxels."\
                         "Treating as-if it is not cached.");
        }
    }
    
    return boost::none;
}

void MapRegion::store(Morton3 key, const Array3D<Voxel> &voxels)
{
    _dataStore.store((size_t)key, _serializer.store(voxels));
}
