//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const AABB &box, const glm::ivec3 &res)
 : _box(box),
   _res(res),
   _cellDim(box.extent.x * 2.0f / res.x,
            box.extent.y * 2.0f / res.y,
            box.extent.z * 2.0f / res.z),
   _chunks(box, res / CHUNK_SIZE)
{}

const Voxel& VoxelData::get(const glm::vec3 &p) const
{
    const MaybeChunk &maybeChunk = _chunks.get(p);
    const Voxel &voxel = maybeChunk->get(p);
    return voxel;
}

Voxel& VoxelData::mutableReference(const glm::vec3 &p)
{
    MaybeChunk &maybeChunk = _chunks.mutableReference(p);
    Voxel &voxel = maybeChunk->mutableReference(p);
    return voxel;
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    MaybeChunk &maybeChunk = _chunks.mutableReference(p);
    
    // If the chunk does not exist then create it now.
    if (!maybeChunk) {
        AABB chunkBoundingBox = _chunks.cellAtPoint(p);
        glm::ivec3 numChunks = _chunks.gridResolution();
        glm::ivec3 chunkRes = _res / numChunks;
        maybeChunk.emplace(chunkBoundingBox, chunkRes);
    }
    
    maybeChunk->set(p, object);
}

glm::vec3 VoxelData::cellDimensions() const
{
    return _cellDim;
}

AABB VoxelData::boundingBox() const
{
    return _box;
}

glm::ivec3 VoxelData::gridResolution() const
{
    return _res;
}

const GridView<Voxel> VoxelData::getView(const AABB &region) const
{
    if (!inbounds(region)) {
        throw OutOfBoundsException();
    }
    return GridView<Voxel>(*this, region);
}

GridViewMutable<Voxel> VoxelData::getView(const AABB &region)
{
    if (!inbounds(region)) {
        throw OutOfBoundsException();
    }
    return GridViewMutable<Voxel>(*this, region);
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
    
    // AFOX_TODO: Use direct array access here.
    dst.mutableForEachCell(adjustedRegion, [&](const AABB &cell){
        return get(cell.center);
    });
    
    return dst;
}
