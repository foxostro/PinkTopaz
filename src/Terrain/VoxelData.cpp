//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"
#include "Grid/GridIndexerRange.hpp"

VoxelData::VoxelData(std::shared_ptr<spdlog::logger> log,
                     std::unique_ptr<VoxelDataGenerator> &&source,
                     unsigned chunkSize,
                     std::unique_ptr<MapRegionStore> &&mapRegionStore)
 : GridIndexer(source->boundingBox(), source->gridResolution()),
   _log(log),
   _source(std::move(source)),
   _chunks(log,
           _source->boundingBox(),
           _source->gridResolution(),
           chunkSize,
           std::move(mapRegionStore),
           [=](const AABB &cell, Morton3 index){
               return createNewChunk(cell, index);
           })
{}

Array3D<Voxel> VoxelData::load(const AABB &region)
{
    return _chunks.load(region);
}

void VoxelData::modify(TerrainOperation &operation)
{
    const AABB region = operation.getAffectedRegion();
    auto data = _chunks.load(region);
    operation.perform(data);
    _chunks.store(data);
}

void VoxelData::setWorkingSet(const AABB &workingSet)
{
    _chunks.setWorkingSet(workingSet);
}

std::unique_ptr<PersistentVoxelChunks::Chunk> VoxelData::createNewChunk(const AABB &cell, Morton3 index)
{
    return std::make_unique<PersistentVoxelChunks::Chunk>(_source->copy(cell));
}
