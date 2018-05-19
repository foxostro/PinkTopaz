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

void TransactedVoxelData::readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn)
{
    const AABB lockedRegion = boundingBox().intersect(region);
    auto mutex = _lockArbitrator.readerMutex(lockedRegion);
    std::lock_guard<decltype(mutex)> lock(mutex);
    _source->readerTransaction(lockedRegion, fn);
}

void TransactedVoxelData::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    const AABB lockedRegion = boundingBox().intersect(_source->getAccessRegionForOperation(operation));
    
    {
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
        _source->writerTransaction(operation);
    }
    
    onWriterTransaction(lockedRegion);
}

void TransactedVoxelData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
}
