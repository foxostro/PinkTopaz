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
#if TransactedVoxelDataUsesBigLock
    std::lock_guard<std::mutex> lock(_mutex);
#else
    const AABB lockedRegion = _source->getSunlightRegion(region);
    auto mutex = _lockArbitrator.writerMutex(lockedRegion);
    std::lock_guard<decltype(mutex)> lock(mutex);
#endif
    
    fn(_source->load(region));
}

void TransactedVoxelData::writerTransaction(TerrainOperation &operation)
{
    const AABB lockedRegion = boundingBox().intersect(_source->getAccessRegionForOperation(operation));
    
    {
#if TransactedVoxelDataUsesBigLock
        std::lock_guard<std::mutex> lock(_mutex);
#else
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
#endif
        
        operation.perform(*_source);
    }
    
    onWriterTransaction(lockedRegion);
}

void TransactedVoxelData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
}
