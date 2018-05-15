//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Grid/SparseGrid.hpp"
#include "Terrain/VoxelDataSource.hpp"
#include "Terrain/MapRegionStore.hpp"
#include "TaskDispatcher.hpp"

// A block of voxels in space.
class VoxelData : public VoxelDataSource
{
public:
    // Default Destructor.
    ~VoxelData() = default;
    
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // source -- The source provides initial voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // dispatcher -- Thread pool to use for asynchronous tasks within VoxelData.
    VoxelData(std::unique_ptr<VoxelDataSource> &&source,
              unsigned chunkSize,
              std::unique_ptr<MapRegionStore> &&mapRegionStore,
              const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // VoxelData may evict chunks to keep the total chunk count under this
    // limit. Pass in at least the expected number of chunks in the working set.
    void setChunkCountLimit(unsigned chunkCountLimit);
    
    // Loads a copy of the contents of the specified sub-region of the grid to
    // an Array3D and returns that. May fault in missing voxels to satisfy the
    // request.
    Array3D<Voxel> load(const AABB &region) override;
    
    // Stores the contents of the specified array of voxels to the grid.
    virtual void store(const Array3D<Voxel> &voxels);
    
private:
    using Chunk = Array3D<Voxel>;
    using ChunkPtr = std::shared_ptr<Chunk>;
    
    std::unique_ptr<VoxelDataSource> _source;
    SparseGrid<ChunkPtr> _chunks;
    std::unique_ptr<MapRegionStore> _mapRegionStore;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    
    // Gets the chunk, creating it if necessary.
    ChunkPtr get(const AABB &bbox, Morton3 index);
};

#endif /* VoxelData_hpp */
