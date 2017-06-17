//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
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
    // Get an AABB which covers the cells which intersect `region'.
    const glm::vec3 halfCellDim = cellDimensions() * 0.5f;
    const glm::vec3 mins = cellCenterAtPoint(region.mins()) - halfCellDim;
    const glm::vec3 maxs = cellCenterAtPoint(region.maxs()) + halfCellDim;
    const glm::vec3 center = (maxs + mins) * 0.5f;
    const glm::vec3 extent = (maxs - mins) * 0.5f;
    const AABB adjustedRegion = {center, extent};
    
    const glm::ivec3 res = countCellsInRegion(adjustedRegion);
    
    Array3D<Voxel> dst(adjustedRegion, res);
    assert(dst.inbounds(region));
    
    dst.mutableForEachCell(adjustedRegion, [&](const AABB &cell){
        return get(cell.center);
    });
    
    return dst;
}

VoxelData::MaybeChunk& VoxelData::chunkAtPoint(const glm::vec3 &p) const
{
    // AFOX_TODO: Need a better way to lock chunks. This effectively serializes all chunk generation.
    std::lock_guard<std::mutex> lock(_lockChunks);
    
    MaybeChunk &maybeChunk = _chunks.mutableReference(p);
    
    // If the chunk does not exist then create it now. The initial contents of
    // the chunk are filled using the generator.
    if (!maybeChunk) {
        AABB chunkBoundingBox = _chunks.cellAtPoint(p);
        glm::ivec3 numChunks = _chunks.gridResolution();
        glm::ivec3 chunkRes = gridResolution() / numChunks;
        maybeChunk.emplace(chunkBoundingBox, chunkRes);
        maybeChunk->mutableForEachCell(chunkBoundingBox, [&](const AABB &cell){
            return _generator.get(cell.center);
        });
    }
    
    assert(maybeChunk);
    return maybeChunk;
}
