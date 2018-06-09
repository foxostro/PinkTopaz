//
//  TransactedVoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#include "Terrain/TransactedVoxelData.hpp"

TransactedVoxelData::TransactedVoxelData(std::unique_ptr<VoxelData> &&source)
 : GridIndexer(source->boundingBox(), source->gridResolution()),
   _source(std::move(source))
{}

void TransactedVoxelData::readerTransaction(const AABB &region, std::function<void(Array3D<Voxel> &&data)> fn)
{
    const AABB lockedRegion = _source->getSunlightRegion(region);
    auto mutex = _lockArbitrator.writerMutex(lockedRegion);
    std::scoped_lock lock(mutex);
    fn(_source->load(region));
}

void TransactedVoxelData::readerTransaction(const std::vector<AABB> regions,
                                            std::function<void(size_t index, Array3D<Voxel> &&data)> fn)
{
    AABB lockedRegion = _source->getSunlightRegion(regions.front());
    for (const AABB &region : regions) {
        lockedRegion = lockedRegion.unionBox(_source->getSunlightRegion(region));
    }
    auto mutex = _lockArbitrator.writerMutex(lockedRegion);
    std::scoped_lock lock(mutex);
    
    for (size_t i = 0; i < regions.size(); ++i) {
        fn(i, _source->load(regions[i]));
    }
}

void TransactedVoxelData::writerTransaction(TerrainOperation &operation)
{
    const AABB lockedRegion = boundingBox().intersect(_source->getAccessRegionForOperation(operation));
    
    {
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::scoped_lock lock(mutex);
        operation.perform(*_source);
    }
    
    onWriterTransaction(lockedRegion);
}
