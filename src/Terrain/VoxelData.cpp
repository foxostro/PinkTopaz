//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#include "Terrain/VoxelData.hpp"

VoxelData::VoxelData(const std::shared_ptr<VoxelDataGenerator> &generator,
                     unsigned chunkSize)
 : _generator(generator),
   _chunks(generator->boundingBox(), _generator->gridResolution() / (int)chunkSize)
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
    return get(cellCenterAtCellCoords(cellCoords));
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
    return mutableReference(cellCenterAtCellCoords(cellCoords));
}

glm::vec3 VoxelData::cellDimensions() const
{
    return _generator->cellDimensions();
}

AABB VoxelData::boundingBox() const
{
    return _generator->boundingBox();
}

glm::ivec3 VoxelData::gridResolution() const
{
    return _generator->gridResolution();
}

Array3D<Voxel> VoxelData::copy(const AABB &region) const
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
    
    // The chunks themselves have already been locked by the VoxelDataStore,
    // but we'll still need to protect accesses to `_chunks'. Let's collect the
    // chunks under the lock and then process the chunks afterward.
    std::vector<std::pair<AABB, std::reference_wrapper<MaybeChunk>>> chunks;
    {
        std::lock_guard<std::mutex> lock(_lockChunks);
        _chunks.mutableForEachCell(region, [&](const AABB &chunkBoundingBox,
                                               Morton3 chunkIndex,
                                               MaybeChunk &maybeChunk){
            auto pair = std::make_pair(chunkBoundingBox, std::reference_wrapper<MaybeChunk>(maybeChunk));
            chunks.push_back(pair);
        });
    }
    
    // Iterate over all chunks in the region.
    for (auto &pair : chunks) {
        const auto &chunkBoundingBox = pair.first;
        MaybeChunk &maybeChunk = pair.second;
        
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
            dst.mutableReference(cell.center) = voxel;
        });
    }
    
    return dst;
}

VoxelData::MaybeChunk& VoxelData::chunkAtPoint(const glm::vec3 &p) const
{
    // The chunk itself will already have been locked by the VoxelDataStore at
    // this point. We only need to protect access to `_chunks' itself.
    MaybeChunk *pMaybeChunk;
    {
        std::lock_guard<std::mutex> lock(_lockChunks);
        pMaybeChunk = &_chunks.mutableReference(p);
    }
    emplaceChunkIfNecessary(p, *pMaybeChunk);
    return *pMaybeChunk;
}

void VoxelData::emplaceChunkIfNecessary(const glm::vec3 &p,
                                        MaybeChunk &maybeChunk) const
{
    // If the chunk does not exist then create it now. The initial contents of
    // the chunk are filled using the generator.
    if (!maybeChunk) {
        maybeChunk.emplace(_generator->copy(_chunks.cellAtPoint(p)));
    }
    
    assert(maybeChunk);
}
