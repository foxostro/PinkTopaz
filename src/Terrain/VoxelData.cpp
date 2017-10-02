//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const GeneratorPtr &gen,
                     unsigned chunkSize,
                     const std::shared_ptr<TaskDispatcher> &dispatcher)
 : GridIndexer(gen->boundingBox(), gen->gridResolution()),
   _generator(gen),
   _chunks(gen->boundingBox(), gen->gridResolution() / (int)chunkSize),
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
    assert(dst.inbounds(region));
    
    // Asynchronously fetch all the chunks in the region.
    std::vector<boost::future<ChunkPtr>> futures;
    for (const auto &cellCoords : _chunks.slice(adjustedRegion)) {
        auto f = _dispatcher->async([this, cellCoords]{
            const Morton3 index = _chunks.indexAtCellCoords(cellCoords);
            const AABB chunkBoundingBox = _chunks.cellAtCellCoords(cellCoords);
            ChunkPtr chunk = get(chunkBoundingBox, index);
            return chunk;
        });
        futures.push_back(std::move(f));
    }
    boost::wait_for_all(futures.begin(), futures.end());
    
    // Copy chunk contents into the destination array.
    for (boost::future<ChunkPtr> &future : futures) {
        ChunkPtr chunk = future.get();
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunk->boundingBox().intersect(region);
        for (const auto &voxelCoords : chunk->slice(subRegion)) {
            const auto voxelCenter = chunk->cellCenterAtCellCoords(voxelCoords);
            dst.mutableReference(voxelCenter) = chunk->reference(voxelCoords);
        }
    }
    
    return dst;
}

void VoxelData::store(const Array3D<Voxel> &voxels)
{
    const AABB region = voxels.boundingBox();
    
    for (const auto &chunkCellCoords : _chunks.slice(region)) {
        const AABB chunkBoundingBox = cellAtCellCoords(chunkCellCoords);
        const Morton3 chunkIndex = indexAtCellCoords(chunkCellCoords);
        ChunkPtr chunk = get(chunkBoundingBox, chunkIndex);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunkBoundingBox.intersect(region);
        for (const auto &voxelCellCoords : chunk->slice(subRegion)) {
            Voxel &dst = chunk->mutableReference(voxelCellCoords);
            
            const auto cellCenter = chunk->cellCenterAtCellCoords(voxelCellCoords);
            const Voxel &src = voxels.reference(cellCenter);
            
            dst = src;
        }
    }
}

VoxelData::ChunkPtr VoxelData::get(const AABB &cell, Morton3 index)
{
    return _chunks.get(index, [=]{
        auto voxels = _generator->copy(cell);
        return std::make_shared<Chunk>(std::move(voxels));
    });
}
