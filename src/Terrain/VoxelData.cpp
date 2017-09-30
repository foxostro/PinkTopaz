//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const GeneratorPtr &gen, unsigned chunkSize)
 : GridIndexer(gen->boundingBox(), gen->gridResolution()),
   _generator(gen),
   _chunks(gen->boundingBox(), gen->gridResolution() / (int)chunkSize)
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
    
    _chunks.forEachCell(region, [&](const AABB &boundingBox, Morton3 index){
        ChunkPtr chunk = get(boundingBox, index);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = boundingBox.intersect(region);
        chunk->forEachCell(subRegion, [&](const AABB &cell,
                                          Morton3 voxelIndex,
                                          const Voxel &voxel){
            dst.mutableReference(cell.center) = voxel;
        });
    });
    
    return dst;
}

void VoxelData::store(const Array3D<Voxel> &voxels)
{
    const AABB region = voxels.boundingBox();
    
    _chunks.forEachCell(region, [&](const AABB &boundingBox, Morton3 index){
        ChunkPtr chunk = get(boundingBox, index);
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = boundingBox.intersect(region);
        chunk->mutableForEachCell(subRegion, [&](const AABB &cell,
                                                 Morton3 voxelIndex,
                                                 Voxel &voxel){
            voxel = voxels.reference(cell.center);
        });
    });
}

VoxelData::ChunkPtr VoxelData::get(const AABB &cell, Morton3 index)
{
    return _chunks.get(index, [=]{
        auto voxels = _generator->copy(cell);
        return std::make_shared<Chunk>(std::move(voxels));
    });
}
