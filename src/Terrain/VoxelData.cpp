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

VoxelDataChunk VoxelData::load(const AABB &region)
{
    return _chunks.load(region);
}

void VoxelData::editSingleVoxel(const glm::vec3 &point, const Voxel &value)
{
    const AABB chunkBoundingBox = _chunks.getChunkIndexer().cellAtPoint(point);
    VoxelDataChunk chunk = _chunks.load(chunkBoundingBox);
    chunk.set(point, value);
    _chunks.store(chunk);
}

void VoxelData::setWorkingSet(const AABB &workingSet)
{
    _chunks.setWorkingSet(workingSet);
}

std::unique_ptr<VoxelDataChunk> VoxelData::createNewChunk(const AABB &cell, Morton3 index)
{
    if (cell.center.y > 64.f || cell.center.y < 0.f) {
        const auto adjusted = _source->snapRegionToCellBoundaries(cell);
        const auto res = _source->countCellsInRegion(adjusted);
        if (cell.center.y < 0.f) {
            return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createGroundChunk(adjusted, res));
        } else {
            return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createSkyChunk(adjusted, res));
        }
    } else {
        return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createArrayChunk(_source->copy(cell)));
    }
}
