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

void VoxelDataStore::readerTransaction(const AABB &region, const Reader &fn) const
{
    // AFOX_TODO: What if we want to copy the region to an Array3D instead
    // of using a GridView?
    PROFILER(VoxelDataStoreReader);
    ConcurrentGridMutable<Voxel>::readerTransaction(region, fn);
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    PROFILER(VoxelDataStoreWriter);
    ConcurrentGridMutable<Voxel>::writerTransaction(region, fn);
}
