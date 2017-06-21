//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/VoxelData.hpp"
#include "Profiler.hpp"

VoxelDataStore::VoxelDataStore(const std::shared_ptr<VoxelDataGenerator> &generator,
                               unsigned chunkSize)
 : ConcurrentGridMutable<Voxel>(std::make_unique<VoxelData>(generator, chunkSize),
                                chunkSize)
{}

void VoxelDataStore::readerTransaction(const AABB &region, const ArrayReader &fn) const
{
    PROFILER(VoxelDataStoreReader);
    LockSet locks(locksForRegion(region));
    auto rawPointer = (VoxelData *)_array.get();
    assert(rawPointer != nullptr);
    const Array3D<Voxel> data = rawPointer->copy(region);
    fn(data);
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    PROFILER(VoxelDataStoreWriter);
    ConcurrentGridMutable<Voxel>::writerTransaction(region, fn);
}
