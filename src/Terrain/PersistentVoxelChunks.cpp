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
                                             std::function<std::unique_ptr<VoxelDataChunk>(const AABB &cell, Morton3 index)> factory)
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

Array3D<Voxel> PersistentVoxelChunks::loadSubRegion(const AABB &region)
{
    // Adjust the region so that it includes the full extent of all voxels that
    // fall within it. For example, the region may only pass through a portion
    // of some voxels on the edge, but the adjusted region should include all
    // of those voxels.
    const AABB adjustedRegion = snapRegionToCellBoundaries(region);
    
    // Construct the destination array.
    const glm::ivec3 res = countCellsInRegion(adjustedRegion);
    Array3D<Voxel> dst(adjustedRegion, res);
    
    // Fetch all the chunks in the region.
    for (auto cellCoords : slice(_chunks, adjustedRegion)) {
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(cellCoords);
        VoxelDataChunk chunk = load(chunkBoundingBox);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunk.boundingBox().intersect(adjustedRegion);
        for (const auto voxelCoords : slice(chunk, subRegion)) {
            const auto voxelCenter = chunk.cellCenterAtCellCoords(voxelCoords);
            dst.mutableReference(voxelCenter) = chunk.get(voxelCoords);
        }
    }
    
    return dst;
}

void PersistentVoxelChunks::storeSubRegion(const Array3D<Voxel> &voxels)
{
    const AABB region = voxels.boundingBox();
    
    for (const auto chunkCellCoords : slice(_chunks, region)) {
        const AABB chunkBoundingBox = _chunks.cellAtCellCoords(chunkCellCoords);
        VoxelDataChunk chunk = load(chunkBoundingBox);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunkBoundingBox.intersect(region);
        for (const auto voxelCellCoords : slice(chunk, subRegion)) {
            const auto cellCenter = chunk.cellCenterAtCellCoords(voxelCellCoords);
            const Voxel &src = voxels.reference(cellCenter);
            chunk.set(voxelCellCoords, src);
        }
        
        store(chunk);
    }
}

VoxelDataChunk PersistentVoxelChunks::load(const AABB &region)
{
    const glm::ivec3 chunkCellCoords = _chunks.cellCoordsAtPoint(region.center);
    const AABB chunkBoundingBox = _chunks.cellAtCellCoords(chunkCellCoords);
    if (chunkBoundingBox != region) {
        throw OutOfBoundsException(fmt::format("OutOfBoundsException\n"\
                                               "region={}\n"\
                                               "chunkBoundingBox={}",
                                               region, chunkBoundingBox));
    }
    const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
    auto chunkPtr = get(chunkBoundingBox, chunkIndex);
    assert(chunkPtr);
    const VoxelDataChunk &chunk = *chunkPtr;
    return chunk;
}

void PersistentVoxelChunks::store(const VoxelDataChunk &chunkToStore)
{
    const AABB region = chunkToStore.boundingBox();
    
    const glm::ivec3 chunkCellCoords = _chunks.cellCoordsAtPoint(region.center);
    const AABB chunkBoundingBox = _chunks.cellAtCellCoords(chunkCellCoords);
    if (chunkBoundingBox != region) {
        throw OutOfBoundsException();
    }
    const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
    
    _chunks.set(chunkIndex, std::make_shared<VoxelDataChunk>(chunkToStore));
    
    // Save the modified chunk back to disk.
    _mapRegionStore->store(chunkBoundingBox, chunkIndex, chunkToStore);
}

std::shared_ptr<VoxelDataChunk>
PersistentVoxelChunks::get(const AABB &cell, Morton3 index)
{
    return _chunks.get(index, [=]{
        auto maybeVoxels = _mapRegionStore->load(cell, index);
        if (maybeVoxels) {
            return std::make_shared<VoxelDataChunk>(*maybeVoxels);
        } else {
            std::shared_ptr<VoxelDataChunk> chunkPtr = _factory(cell, index);
            assert(chunkPtr);
            VoxelDataChunk &chunk = *chunkPtr;
            _mapRegionStore->store(cell, index, chunk); // save to disk
            return chunkPtr;
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
