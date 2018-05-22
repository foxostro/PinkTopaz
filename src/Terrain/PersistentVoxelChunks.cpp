//
//  PersistentVoxelChunks.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#include "Terrain/PersistentVoxelChunks.hpp"
#include "Grid/GridIndexerRange.hpp"

PersistentVoxelChunks::PersistentVoxelChunks(std::shared_ptr<spdlog::logger> log,
                                             const AABB &boundingBox,
                                             const glm::ivec3 gridResolution,
                                             unsigned chunkSize,
                                             std::unique_ptr<MapRegionStore> &&mapRegionStore,
                                             std::function<std::unique_ptr<Chunk>(const AABB &cell, Morton3 index)> factory)
 : GridIndexer(boundingBox, gridResolution),
   _log(log),
   _chunks(boundingBox, gridResolution / (int)chunkSize),
   _mapRegionStore(std::move(mapRegionStore)),
   _factory(factory)
{}

void PersistentVoxelChunks::setWorkingSet(const AABB &workingSet)
{
    glm::ivec3 count = _chunks.countCellsInRegion(workingSet);
    size_t chunkCountLimit = count.x * count.y * count.z;
    _chunks.setCountLimit(chunkCountLimit);
}

PersistentVoxelChunks::Chunk PersistentVoxelChunks::load(const AABB &region)
{
    // Adjust the region so that it includes the full extent of all voxels that
    // fall within it. For example, the region may only pass through a portion
    // of some voxels on the edge, but the adjusted region should include all
    // of those voxels.
    const AABB adjustedRegion = snapRegionToCellBoundaries(region);
    
    // Construct the destination array.
    const glm::ivec3 res = countCellsInRegion(adjustedRegion);
    Chunk dst(adjustedRegion, res);
    
    // Fetch all the chunks in the region.
    for (auto cellCoords : slice(_chunks, adjustedRegion)) {
        const Morton3 index = _chunks.indexAtCellCoords(cellCoords);
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(cellCoords);
        auto chunk = get(chunkBoundingBox, index);
        
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

void PersistentVoxelChunks::store(const Chunk &voxels)
{
    const AABB region = voxels.boundingBox();
    
    for (const auto chunkCellCoords : slice(_chunks, region)) {
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(chunkCellCoords);
        const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
        auto chunk = get(chunkBoundingBox, chunkIndex);
        
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

std::shared_ptr<PersistentVoxelChunks::Chunk>
PersistentVoxelChunks::get(const AABB &cell, Morton3 index)
{
    return _chunks.get(index, [=]{
        auto maybeVoxels = _mapRegionStore->load(cell, index);
        if (maybeVoxels) {
            return std::make_shared<Chunk>(*maybeVoxels);
        } else {
            std::shared_ptr<Chunk> chunk = _factory(cell, index);
            _mapRegionStore->store(cell, index, *chunk); // save to disk
            return chunk;
        }
    });
}

void PersistentVoxelChunks::invalidate(const AABB &region)
{
    for (const auto chunkCellCoords : slice(_chunks, region)) {
        const glm::vec3 chunkCenter = _chunks.cellCenterAtCellCoords(chunkCellCoords);
        const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
        _chunks.invalidate(chunkIndex);
        _mapRegionStore->invalidate(chunkCenter, chunkIndex);
    }
}
