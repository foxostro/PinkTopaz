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
                     std::unique_ptr<VoxelDataSource> &&source,
                     unsigned chunkSize,
                     std::unique_ptr<MapRegionStore> &&mapRegionStore)
 : VoxelDataSource(source->boundingBox(), source->gridResolution()),
   _log(log),
   _source(std::move(source)),
   _chunks(_source->boundingBox(), _source->gridResolution() / (int)chunkSize),
   _mapRegionStore(std::move(mapRegionStore))
{}

void VoxelData::readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn)
{
    auto mutex = _lockArbitrator.readerMutex(region);
    std::lock_guard<decltype(mutex)> lock(mutex);
    const Array3D<Voxel> data = load(region);
    fn(data);
}

void VoxelData::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    const AABB lockedRegion = boundingBox().intersect(operation->getAffectedRegion());
    
    {
        auto mutex = _lockArbitrator.writerMutex(lockedRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
        Array3D<Voxel> data = load(lockedRegion);
        operation->perform(data);
        store(data);
    }
    
    onWriterTransaction(lockedRegion);
}

void VoxelData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
    
    glm::ivec3 count = _chunks.countCellsInRegion(workingSet);
    size_t chunkCountLimit = count.x * count.y * count.z;
    _chunks.setCountLimit(chunkCountLimit);
}

VoxelData::Chunk VoxelData::load(const AABB &region)
{
    // Adjust the region so that it includes the full extent of all voxels that
    // fall within it. For example, the region may only pass through a portion
    // of some voxels on the edge, but the adjusted region should include all
    // of those voxels.
    const AABB adjustedRegion = _source->snapRegionToCellBoundaries(region);
    
    // Construct the destination array.
    const glm::ivec3 res = _source->countCellsInRegion(adjustedRegion);
    Array3D<Voxel> dst(adjustedRegion, res);
    
    // Fetch all the chunks in the region.
    for (auto cellCoords : slice(_chunks, adjustedRegion)) {
        const Morton3 index = _chunks.indexAtCellCoords(cellCoords);
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(cellCoords);
        ChunkPtr chunk = get(chunkBoundingBox, index);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunk->boundingBox().intersect(adjustedRegion);
        for (const auto voxelCoords : slice(*chunk, subRegion)) {
            const auto voxelCenter = chunk->cellCenterAtCellCoords(voxelCoords);
            dst.mutableReference(voxelCenter) = chunk->reference(voxelCoords);
        }
    }

    return dst;
}

void VoxelData::store(const Chunk &voxels)
{
    const AABB region = voxels.boundingBox();
    
    for (const auto chunkCellCoords : slice(_chunks, region)) {
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(chunkCellCoords);
        const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
        ChunkPtr chunk = get(chunkBoundingBox, chunkIndex);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunkBoundingBox.intersect(region);
        for (const auto voxelCellCoords : slice(*chunk, subRegion)) {
            Voxel &dst = chunk->mutableReference(voxelCellCoords);
            
            const auto cellCenter = chunk->cellCenterAtCellCoords(voxelCellCoords);
            const Voxel &src = voxels.reference(cellCenter);
            
            dst = src;
        }
        
        // Save the modified chunk back to disk.
        _mapRegionStore->store(chunkBoundingBox, chunkIndex, *chunk);
    }
}

VoxelData::ChunkPtr VoxelData::get(const AABB &cell, Morton3 index)
{
    return _chunks.get(index, [=]{
        auto maybeVoxels = _mapRegionStore->load(cell, index);
        if (maybeVoxels) {
            return std::make_shared<Chunk>(*maybeVoxels);
        } else {
            ChunkPtr chunk = createNewChunk(cell, index);
            _mapRegionStore->store(cell, index, *chunk); // save to disk
            return chunk;
        }
    });
}

VoxelData::ChunkPtr VoxelData::createNewChunk(const AABB &cell, Morton3 index)
{
    ChunkPtr chunk;
    _source->readerTransaction(cell, [&](const Array3D<Voxel> &voxels) {
        chunk = std::make_shared<Chunk>(voxels);
    });
    return chunk;
}
