//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Terrain/PersistentVoxelChunks.hpp"
#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/TerrainOperation.hpp"

#include <spdlog/spdlog.h>
#include <memory>
#include <functional>

// A block of voxels in space.
class VoxelData : public GridIndexer
{
public:
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source -- The source provides initial voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // mapRegionStore -- The map file in which to persist chunks.
    VoxelData(std::shared_ptr<spdlog::logger> log,
              std::unique_ptr<VoxelDataGenerator> &&source,
              unsigned chunkSize,
              std::unique_ptr<MapRegionStore> &&mapRegionStore);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // fn -- Closure which will be doing the reading.
    void readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn);
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(TerrainOperation &operation);
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet);
    
private:
    std::shared_ptr<spdlog::logger> _log;
    std::unique_ptr<VoxelDataGenerator> _source;
    PersistentVoxelChunks _chunks;
    
    std::unique_ptr<PersistentVoxelChunks::Chunk> createNewChunk(const AABB &cell);
};

#endif /* VoxelData_hpp */
