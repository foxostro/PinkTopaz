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
    // source -- The source provides initial, unlit voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    // dispatcher -- Thread pool to use for asynchronous tasks.
    SunlightData(std::unique_ptr<VoxelDataSource> &&source,
                 unsigned chunkSize,
                 std::unique_ptr<MapRegionStore> &&mapRegionStore,
                 const std::shared_ptr<TaskDispatcher> &dispatcher);
    
protected:
    // Returns a new chunk for the corresponding region of space.
    // The chunk is populated using data gathered from the underlying source.
    // boundingBox -- The bounding box of the chunk.
    // index -- A unique index to identify the chunk in the sparse grid.
    ChunkPtr createNewChunk(const AABB &boundingBox, Morton3 index) override;
    
    // Return the region of voxels affected by a a given operation.
    // The region of voxels invalidated by a change made by some operation may
    // be larger than the region directly modified by that operation.
    // For example, an edit a single block may affect light values for blocks
    // some distance away.
    AABB getAffectedRegionForOperation(const std::shared_ptr<TerrainOperation> &operation) override;
    
private:
    std::unique_ptr<Noise> _noiseSource;
};

#endif /* SunlightData_hpp */
