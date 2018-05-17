//
//  SunlightData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#ifndef SunlightData_hpp
#define SunlightData_hpp

#include "Terrain/VoxelData.hpp"
#include "Noise/Noise.hpp"

// Voxels with sunlight computation.
class SunlightData : public VoxelData
{
public:
    // No default constructor.
    SunlightData() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // source -- The source provides initial, unlit voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // dispatcher -- Thread pool to use for asynchronous tasks.
    SunlightData(std::shared_ptr<spdlog::logger> log,
                 std::unique_ptr<VoxelDataSource> &&source,
                 unsigned chunkSize,
                 std::unique_ptr<MapRegionStore> &&mapRegionStore,
                 const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // Perform an atomic transaction as a "writer" with read-write access to
    // the underlying voxel data in the specified region.
    // operation -- Describes the edits to be made.
    void writerTransaction(const std::shared_ptr<TerrainOperation> &operation) override;
    
protected:
    // Returns a new chunk for the corresponding region of space.
    // The chunk is populated using data gathered from the underlying source.
    // boundingBox -- The bounding box of the chunk.
    // index -- A unique index to identify the chunk in the sparse grid.
    ChunkPtr createNewChunk(const AABB &boundingBox, Morton3 index) override;
    
private:
    std::unique_ptr<Noise> _noiseSource;
};

#endif /* SunlightData_hpp */
