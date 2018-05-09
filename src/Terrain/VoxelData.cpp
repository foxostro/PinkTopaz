//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"
#include "Grid/GridIndexerRange.hpp"

VoxelData::VoxelData(const GeneratorPtr &gen,
                     unsigned chunkSize,
                     std::unique_ptr<MapRegionStore> &&mapRegionStore,
                     const std::shared_ptr<TaskDispatcher> &dispatcher)
 : GridIndexer(gen->boundingBox(), gen->gridResolution()),
   _generator(gen),
   _chunks(gen->boundingBox(), gen->gridResolution() / (int)chunkSize),
   _mapRegionStore(std::move(mapRegionStore)),
   _dispatcher(dispatcher)
{}

void VoxelData::setChunkCountLimit(unsigned chunkCountLimit)
{
    _chunks.setCountLimit(chunkCountLimit);
}

Array3D<Voxel> VoxelData::load(const AABB &region)
{
    // Adjust the region so that it includes the full extent of all voxels that
    // fall within it. For example, the region may only pass through a portion
    // of some voxels on the edge, but the adjusted region should include all
    // of those voxels.
    const glm::vec3 cellEx = _generator->cellDimensions() * 0.5f;
    const glm::vec3 mins = _generator->cellCenterAtPoint(region.mins()) - cellEx;
    const glm::vec3 maxs = _generator->cellCenterAtPoint(region.maxs()) + cellEx;
    const glm::vec3 center = (maxs + mins) * 0.5f;
    const glm::vec3 extent = (maxs - mins) * 0.5f;
    const AABB adjustedRegion = {center, extent};
    
    // Construct the destination array.
    const glm::ivec3 res = _generator->countCellsInRegion(adjustedRegion);
    Array3D<Voxel> dst(adjustedRegion, res);
    
    // Asynchronously fetch all the chunks in the region.
    auto futures = _dispatcher->map(slice(_chunks, adjustedRegion), [this, adjustedRegion, &dst](glm::ivec3 cellCoords){
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
    });
    
    waitForAll(futures);

    return dst;
}

void VoxelData::store(const Array3D<Voxel> &voxels)
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
            auto voxels = _generator->copy(cell);
            _mapRegionStore->store(cell, index, voxels); // save to disk
            return std::make_shared<Chunk>(std::move(voxels));
        }
    });
}
