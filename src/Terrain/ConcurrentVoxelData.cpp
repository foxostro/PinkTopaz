//
//  ConcurrentVoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#include "Terrain/ConcurrentVoxelData.hpp"

ConcurrentVoxelData::ConcurrentVoxelData(std::unique_ptr<VoxelDataSource> &&source)
 : VoxelDataSource(source->boundingBox(), source->gridResolution()),
   _source(std::move(source))
{
    // Note that we do not have to register with _source->onWriterTransaction
    // beause we have a unique pointer to it and already handle modifications
    // made to it through writerTransaction().
}

void ConcurrentVoxelData::readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn)
{
    const AABB lockedRegion = boundingBox().intersect(region);
    auto mutex = _lockArbitrator.readerMutex(lockedRegion);
    std::lock_guard<decltype(mutex)> lock(mutex);
    _source->readerTransaction(lockedRegion, fn);
}

void ConcurrentVoxelData::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    const AABB lockedRegion = boundingBox().intersect(operation->getAffectedRegion());
    
    {
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
        _source->writerTransaction(operation);
    }
    
    onWriterTransaction(lockedRegion);
}

void ConcurrentVoxelData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
}
