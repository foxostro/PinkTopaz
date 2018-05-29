//
//  TransactedVoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#include "Terrain/TransactedVoxelData.hpp"

TransactedVoxelData::TransactedVoxelData(std::unique_ptr<SunlightData> &&source)
 : GridIndexer(source->boundingBox(), source->gridResolution()),
   _source(std::move(source))
{}

void TransactedVoxelData::readerTransaction(const AABB &region, std::function<void(Array3D<Voxel> &&data)> fn)
{
    const AABB lockedRegion = _source->getSunlightRegion(region);
    auto mutex = _lockArbitrator.readerMutex(lockedRegion);
    std::lock_guard<decltype(mutex)> lock(mutex);
    // TODO: If all chunks in specified region are Sky/Ground then we can avoid an expensive array copy and return a Sky/Ground typed VoxelDataChunk here.
    fn(_source->load(region));
}

void TransactedVoxelData::writerTransaction(TerrainOperation &operation)
{
    const AABB lockedRegion = boundingBox().intersect(_source->getAccessRegionForOperation(operation));
    
    {
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
        operation.perform(*_source);
    }
    
    onWriterTransaction(lockedRegion);
}

void TransactedVoxelData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
}
