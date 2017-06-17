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

Voxel& VoxelData::mutableReference(const glm::vec3 &p)
{
    MaybeChunk &maybeChunk = chunkAtPoint(p);
    assert(maybeChunk);
    Voxel &voxel = maybeChunk->mutableReference(p);
    return voxel;
}

void VoxelData::set(const glm::vec3 &p, const Voxel &object)
{
    MaybeChunk &maybeChunk = chunkAtPoint(p);
    assert(maybeChunk);
    maybeChunk->set(p, object);
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
