//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const std::shared_ptr<VoxelDataGenerator> &gen,
                     unsigned chunkSize)
 : GridIndexer(gen->boundingBox(), gen->gridResolution()),
   _generator(gen),
   _chunks(gen->boundingBox(), gen->gridResolution() / (int)chunkSize)
{}

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
    
    // Count the number of voxels in the adjusted region. This will be the grid
    // resolution of the destination array.
    const glm::ivec3 res = _generator->countCellsInRegion(adjustedRegion);
    
    // Construct the destination array.
    Array3D<Voxel> dst(adjustedRegion, res);
    assert(dst.inbounds(region));
    
    // Iterate over all chunks in the region.
    _chunks.mutableForEachCell(region, [&](const AABB &chunkBoundingBox,
                                           Morton3 chunkIndex,
                                           MaybeChunk &maybeChunk){
        // If the chunk does not exist then create it now. The initial contents of
        // the chunk are filled using the generator.
        if (!maybeChunk) {
            maybeChunk.emplace(_generator->copy(_chunks.cellAtPoint(chunkBoundingBox.center)));
        }
        
        // It is entirely possible that the sub-region is not the full size of
        // the chunk. Iterate over chunk voxels that fall within the region.
        // Copy each of those voxels into the destination array.
        const AABB subRegion = chunkBoundingBox.intersect(region);
        maybeChunk->forEachCell(subRegion, [&](const AABB &cell,
                                               Morton3 voxelIndex,
                                               const Voxel &voxel){
            
            // AFOX_TODO: I can reduce calls to indexAtPoint() by being clever
            // with grid coordinates. These calls account for 10% of all time
            // spent in rebuildNextMesh(), and 32% of the time spent in copy().
            dst.mutableReference(cell.center) = voxel;
        });
    });
    
    return dst;
}

void VoxelData::store(const Array3D<Voxel> &voxels)
{
    assert(!"stub");
}
