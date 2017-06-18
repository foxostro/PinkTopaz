//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const VoxelDataGenerator &generator)
 : _generator(generator),
   _chunks(generator.boundingBox(), _generator.gridResolution() / CHUNK_SIZE)
{}

const Voxel& VoxelData::get(const glm::vec3 &p) const
{
    const MaybeChunk &maybeChunk = chunkAtPoint(p);
    assert(maybeChunk);
    const Voxel &voxel = maybeChunk->get(p);
    return voxel;
}

const Voxel& VoxelData::get(const glm::ivec3 &cellCoords) const
{
    // Could potentially be more elegant. Let's avoid any confusion about which
    // grid coords we're using by converting back to world space immediately.
    return get(worldPosAtCellCoords(cellCoords));
}

Voxel& VoxelData::mutableReference(const glm::vec3 &p)
{
    MaybeChunk &maybeChunk = chunkAtPoint(p);
    assert(maybeChunk);
    Voxel &voxel = maybeChunk->mutableReference(p);
    return voxel;
}

Voxel& VoxelData::mutableReference(const glm::ivec3 &cellCoords)
{
    // Could potentially be more elegant. Let's avoid any confusion about which
    // grid coords we're using by converting back to world space immediately.
    return mutableReference(worldPosAtCellCoords(cellCoords));
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    MaybeChunk &maybeChunk = chunkAtPoint(p);
    assert(maybeChunk);
    maybeChunk->set(p, object);
}

void VoxelData::set(const glm::ivec3 &cellCoords, const Voxel &object)
{
    // Could potentially be more elegant. Let's avoid any confusion about which
    // grid coords we're using by converting back to world space immediately.
    set(worldPosAtCellCoords(cellCoords), object);
}

glm::vec3 VoxelData::cellDimensions() const
{
    return _generator.cellDimensions();
}

AABB VoxelData::boundingBox() const
{
    return _generator.boundingBox();
}

glm::ivec3 VoxelData::gridResolution() const
{
    return _generator.gridResolution();
}

Array3D<Voxel> VoxelData::copy(const AABB &region) const
{
    // Adjust the region so that it includes the full extent of all voxels that
    // fall within it. For example, the region may only pass through a portion
    // of some voxels on the edge, but the adjusted region should include all
    // of those voxels.
    const glm::vec3 cellEx = _generator.cellDimensions() * 0.5f;
    const glm::vec3 mins = _generator.cellCenterAtPoint(region.mins()) - cellEx;
    const glm::vec3 maxs = _generator.cellCenterAtPoint(region.maxs()) + cellEx;
    const glm::vec3 center = (maxs + mins) * 0.5f;
    const glm::vec3 extent = (maxs - mins) * 0.5f;
    const AABB adjustedRegion = {center, extent};
    
    // Count the number of voxels in the adjusted region. This will be the grid
    // resolution of the destination array.
    const glm::ivec3 res = _generator.countCellsInRegion(adjustedRegion);
    
    // Construct the destination array.
    Array3D<Voxel> dst(adjustedRegion, res);
    assert(dst.inbounds(region));
    
#if 0
    // Iterate over all chunks in the region. For each of those chunks, build
    // the chunk if it is missing and then iterate over voxels in the chunk that
    // fall within the region. For each of those voxels, copy them into the
    // destination array.
    std::lock_guard<std::mutex> lock(_lockChunks);
    _chunks.mutableForEachCell(region, [&](const AABB &chunkBoundingBox,
                                           Morton3 chunkIndex,
                                           MaybeChunk &maybeChunk){
        
        // Build the chunk if it is missing.
        emplaceChunkIfNecessary(chunkBoundingBox.center, maybeChunk);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunkBoundingBox.intersect(region);
        maybeChunk->forEachCell(subRegion, [&](const AABB &cell,
                                               Morton3 voxelIndex,
                                               const Voxel &voxel){
            
            // AFOX_TODO: I can reduce calls to indexAtPoint() by being clever
            // with grid coordinates.
            dst.set(cell.center, voxel);
        });
    });
#else
    dst.mutableForEachCell(adjustedRegion, [&](const AABB &cell, Morton3 index, Voxel &value){
        // We need to use get(vec3) because the index is only valid within
        // this one chunk.
        value = get(cell.center);
    });
#endif
    
    return dst;
}

VoxelData::MaybeChunk& VoxelData::chunkAtPoint(const glm::vec3 &p) const
{
    // AFOX_TODO: Need a better way to lock chunks. This effectively serializes all chunk generation.
    std::lock_guard<std::mutex> lock(_lockChunks);
    MaybeChunk &maybeChunk = _chunks.mutableReference(p);
    emplaceChunkIfNecessary(p, maybeChunk);
    return maybeChunk;
}

void VoxelData::emplaceChunkIfNecessary(const glm::vec3 &p,
                                        MaybeChunk &maybeChunk) const
{
    // If the chunk does not exist then create it now. The initial contents of
    // the chunk are filled using the generator.
    if (!maybeChunk) {
        const AABB chunkBoundingBox = _chunks.cellAtPoint(p);
        glm::ivec3 numChunks = _chunks.gridResolution();
        glm::ivec3 chunkRes = gridResolution() / numChunks;
        maybeChunk.emplace(chunkBoundingBox, chunkRes);
        maybeChunk->mutableForEachCell(chunkBoundingBox, [&](const AABB &cell,
                                                             Morton3 index,
                                                             Voxel &value){
            // We need to use get(vec3) because the index is only valid within
            // this one chunk.
            // AFOX_TODO: A bulk API for getting voxels from the generator would
            // allow is to more efficiently fill the chunk.
            value = _generator.get(cell.center);
        });
    }
    
    assert(maybeChunk);
}
