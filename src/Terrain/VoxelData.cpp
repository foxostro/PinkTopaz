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

const Voxel& VoxelData::get(const glm::vec3 &p, const Voxel &defaultValue) const
{
    // Return the default value when the point is outside the valid region.
    if (!_chunks.inbounds(p)) {
        return defaultValue;
    }
    
    const MaybeChunk &maybeChunk = _chunks.get(p);
    
    // If the chunk doesn't exist then immediately default to the default value.
    if (!maybeChunk) {
        return defaultValue;
    } else {
        const Voxel &voxel = maybeChunk->get(p, defaultValue);
        return voxel;
    }
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    MaybeChunk &maybeChunk = _chunks.mutableReference(p);
    
    // If the chunk does not exist then create it now.
    if (!maybeChunk) {
        AABB chunkBoundingBox = _chunks.cellAtPoint(p);
        glm::ivec3 numChunks = _chunks.getResolution();
        glm::ivec3 chunkRes = _res / numChunks;
        maybeChunk.emplace(chunkBoundingBox, chunkRes);
    }
    
    maybeChunk->set(p, object);
}

glm::vec3 VoxelData::getCellDimensions() const
{
    return _cellDim;
}

AABB VoxelData::getBoundingBox() const
{
    return _box;
}

glm::ivec3 VoxelData::getResolution() const
{
    return _res;
}

const GridView<Voxel> VoxelData::getView(const AABB &region) const
{
    return GridView<Voxel>(*this, region);
}

GridViewMutable<Voxel> VoxelData::getView(const AABB &region)
{
    return GridViewMutable<Voxel>(*this, region);
}
