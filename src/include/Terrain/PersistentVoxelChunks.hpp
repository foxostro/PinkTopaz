//
//  PersistentVoxelChunks.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#ifndef PersistentVoxelChunks_hpp
#define PersistentVoxelChunks_hpp

#include "Grid/SparseGrid.hpp"
#include "Terrain/MapRegionStore.hpp"

#include <spdlog/spdlog.h>

// Store/Load voxel chunks to file.
class PersistentVoxelChunks : public GridIndexer
{
public:
    using Chunk = Array3D<Voxel>;
    
    // No default constructor.
    PersistentVoxelChunks() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // boundingBox -- The region where voxels are defined.
    // gridResolution -- The resolution of the voxel grid.
    // chunkSize -- The size of chunk to use.
    // mapRegionStore -- The map file in which to persist chunks.
    // factory -- Closure to invoke to populate a new chunk.
    PersistentVoxelChunks(std::shared_ptr<spdlog::logger> log,
                          const AABB &boundingBox,
                          const glm::ivec3 gridResolution,
                          unsigned chunkSize,
                          std::unique_ptr<MapRegionStore> &&mapRegionStore,
                          std::function<std::unique_ptr<Chunk>(const AABB &cell, Morton3 index)> factory);
    
    // We may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet);
    
    // Returns a new chunk for the corresponding region of space.
    // The chunk is populated using data gathered from the underlying source.
    // boundingBox -- The bounding box of the chunk.
    // index -- The coordinates of the chunk.
    std::shared_ptr<Chunk> createNewChunk(const AABB &boundingBox, Morton3 index);
    
    // Loads a copy of the contents of the specified sub-region of the grid
    // and returns that. May fault in missing voxels to satisfy the request.
    // Appropriate locks must be held while calling this method.
    Chunk load(const AABB &region);
    
    // Stores the contents of the specified array of voxels to the grid.
    // Appropriate locks must be held while calling this method.
    void store(const Chunk &voxels);
    
    // Returns the chunk, creating it if necessary, but prefering to fetch it
    // from the map region file.
    // boundingBox -- The bounding box of the chunk.
    // index -- A unique index to identify the chunk in the sparse grid.
    std::shared_ptr<Chunk> get(const AABB &boundingBox, Morton3 index);
    
    // Invalidate all chunks in the specified region.
    void invalidate(const AABB &region);
    
private:
    std::shared_ptr<spdlog::logger> _log;
    SparseGrid<std::shared_ptr<Chunk>> _chunks;
    std::unique_ptr<MapRegionStore> _mapRegionStore;
    std::function<std::unique_ptr<Chunk>(const AABB &cell, Morton3 index)> _factory;
};

#endif /* PersistentVoxelChunks_hpp */
