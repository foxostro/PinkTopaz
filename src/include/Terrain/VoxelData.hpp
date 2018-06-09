//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Terrain/PersistentVoxelChunks.hpp"
#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/TerrainOperation.hpp"
#include "TaskDispatcher.hpp"

#include <spdlog/spdlog.h>
#include <queue>

// Terrain voxels in space with flood-fill lighting.
class VoxelData : public GridIndexer
{
public:
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source --  The source procedurally generates unlit voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // mapRegionStore -- The map file in which to persist chunks.
    // dispatcher -- Dispatcher used to fetch voxels data from the generator.
    //               This permits parallel fetch and generation of voxel data.
    VoxelData(std::shared_ptr<spdlog::logger> log,
              std::unique_ptr<VoxelDataGenerator> &&source,
              unsigned chunkSize,
              std::unique_ptr<MapRegionStore> &&mapRegionStore,
              const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // Loads the chunk at the specified region of space.
    // The specified region of space is not required to exactly match the
    // position and size of one of the chunks used internally by the sunlight
    // data grid. This can be an arbitrary region of space with the bounds of
    // the grid.
    // May fault in missing voxels to satisfy the request.
    Array3D<Voxel> load(const AABB &region);
    
    // Edits a single voxel.
    void editSingleVoxel(const glm::vec3 &point, const Voxel &value);
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    AABB getAccessRegionForOperation(TerrainOperation &operation);
    
    // Accesses to voxel data may require loading surrounding chunks and
    // performing sunlight propagation. This is a conservative estimate of the
    // region where changes may occur. Use this to determine the region which
    // needs to be locked during the access.
    AABB getSunlightRegion(AABB sunlightRegion) const;
    
private:
    // Logger to use.
    std::shared_ptr<spdlog::logger> _log;
    
    // Procedurally generates voxel data on demand.
    std::unique_ptr<VoxelDataGenerator> _source;
    
    // Helper object for storing and persisting chunks of voxel data.
    // Generally, VoxelData encapsulates the concept of chunks. Clients can
    // remain unaware that voxel data is stored in chunks internally.
    PersistentVoxelChunks _chunks;
    
    // Dispatcher used to fetch voxels data from the generator. We do this
    // to provide the opportunity for the voxel data generator to generate the
    // chunks in parallel.
    std::shared_ptr<TaskDispatcher> _dispatcher;
    
    // Returns a new chunk for the specified cell.
    // cell -- The bounding box of the chunk.
    // index -- An index into the chunk grid corresponding to the cell.
    std::unique_ptr<VoxelDataChunk> createNewChunk(const AABB &cell, Morton3 index);
};

#endif /* VoxelData_hpp */
