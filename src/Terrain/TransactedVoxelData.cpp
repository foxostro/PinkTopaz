//
//  TransactedVoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/TransactedVoxelData.hpp"

TransactedVoxelData::TransactedVoxelData(std::unique_ptr<VoxelData> &&voxelData)
 : GridIndexer(voxelData->boundingBox(), voxelData->gridResolution()),
   _array(std::move(voxelData))
{}

void TransactedVoxelData::readerTransaction(const AABB &region, const Reader &fn) const
{
    auto mutex = _lockArbitrator.readerMutex(region);
    std::lock_guard<decltype(mutex)> lock(mutex);
    const Array3D<Voxel> data = _array->load(region);
    fn(data);
}

void TransactedVoxelData::writerTransaction(TerrainOperation &operation)
{
    const AABB region = operation.getAffectedRegion();
    
    {
        auto mutex = _lockArbitrator.writerMutex(region);
        std::lock_guard<decltype(mutex)> lock(mutex);
        Array3D<Voxel> data = _array->load(region);
        operation.perform(data);
        _array->store(data);
    }
    
    onWriterTransaction(region);
}
