//
//  PersistentVoxelChunks.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/17/18.
//
//

#ifndef PersistentVoxelChunks_hpp
#define PersistentVoxelChunks_hpp

#include "Grid/ConcurrentSparseGrid.hpp"
#include "Terrain/MapRegionStore.hpp"
#include "Terrain/VoxelDataChunk.hpp"
#include "TaskDispatcher.hpp"

#include <spdlog/spdlog.h>

// Store/Load voxel chunks to file.
class PersistentVoxelChunks : public GridIndexer
{
public:
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
                          std::function<std::unique_ptr<VoxelDataChunk>(const AABB &cell, Morton3 index)> factory);
    
    // Returns a new chunk for the corresponding region of space.
    // The chunk is populated using data gathered from the underlying source.
    // boundingBox -- The bounding box of the chunk.
    // index -- The coordinates of the chunk.
    std::shared_ptr<VoxelDataChunk> createNewChunk(const AABB &boundingBox, Morton3 index);
    
    // Copies voxels of the specified sub-region of the grid to an array and
    // returns that. May fault in missing voxels to satisfy the request.
    // The specified region may be any AABB within the bounds of the grid.
    Array3D<Voxel> loadSubRegion(const AABB &region);
    
    // Stores the voxels of the specified sub-region to the grid.
    // Each chunk touched by the specified voxel data is converted to an Array
    // chunk to accomodate the changes.
    // The specified region may be any AABB within the bounds of the grid.
    void storeSubRegion(const Array3D<Voxel> &voxels);
    
    // Loads the chunk at the specified region of space.
    // The specified region of space must exactly match the position and size of
    // one of the chunks used internally by PersistentVoxelChunks.
    // May fault in missing voxels to satisfy the request.
    VoxelDataChunk load(const AABB &region);
    
    // Stores the specified chunk immediately.
    // This chunk must exactly match the position and size of one of the chunks
    // used internally by PersistentVoxelChunks.
    void store(const VoxelDataChunk &voxels);
    
    // Re-saves the chunk for the specified index.
    // The save is done asynchronously on the specified task dispatcher.
    // This is useful when a chunk is retrieved via get() and then modified.
    void store(Morton3 index, const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // Returns the chunk, creating it if necessary, but prefering to fetch it
    // from the map region file.
    // boundingBox -- The bounding box of the chunk.
    // index -- A unique index to identify the chunk in the sparse grid.
    std::shared_ptr<VoxelDataChunk> get(const AABB &boundingBox, Morton3 index);
    
    // Returns the chunk, if it already exists.
    // index -- A unique index to identify the chunk in the sparse grid.
    boost::optional<std::shared_ptr<VoxelDataChunk>>
    getIfExists(Morton3 index);
    
    // Return an indexer for the grid of chunks.
    const GridIndexer& getChunkIndexer() const;
    
private:
    std::shared_ptr<spdlog::logger> _log;
    ConcurrentSparseGrid<std::shared_ptr<VoxelDataChunk>> _chunks;
    std::unique_ptr<MapRegionStore> _mapRegionStore;
    std::function<std::unique_ptr<VoxelDataChunk>(const AABB &cell, Morton3 index)> _factory;
};

#endif /* PersistentVoxelChunks_hpp */
