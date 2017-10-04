//
//  MapRegionStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "Terrain/MapRegionStore.hpp"
#include "FileUtilities.hpp"
#include <boost/filesystem.hpp>

static boost::filesystem::path fileNameForChunk(Morton3 key)
{
    const std::string fileName("Chunk_" + std::to_string((size_t)key) + ".bin");
    return getPrefPath() / fileName;
}

MapRegionStore::MapRegionStore(const AABB &boundingBox, unsigned mapRegionSize)
: GridIndexer(boundingBox, glm::ivec3(boundingBox.extent*2.f) / (int)mapRegionSize)
{}

boost::optional<Array3D<Voxel>> MapRegionStore::load(const AABB &boundingBox, Morton3 key)
{
    auto mutex = _lockArbitrator.readerMutex(boundingBox);
    std::lock_guard<decltype(mutex)> lock(mutex);
    
    const auto maybeBytes = binaryFileContents(fileNameForChunk(key));
    if (maybeBytes) {
        auto chunk = _serializer.load(boundingBox, *maybeBytes);
        return boost::make_optional(chunk);
    } else {
        return boost::none;
    }
}

void MapRegionStore::store(const AABB &boundingBox, Morton3 key, const Array3D<Voxel> &voxels)
{
    auto mutex = _lockArbitrator.writerMutex(boundingBox);
    std::lock_guard<decltype(mutex)> lock(mutex);
    
    const auto bytes = _serializer.store(voxels);
    auto path = fileNameForChunk(key);
    saveBinaryFile(path, bytes);
}
