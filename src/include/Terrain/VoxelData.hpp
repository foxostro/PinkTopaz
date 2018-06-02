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
    VoxelData(std::shared_ptr<spdlog::logger> log,
                 std::unique_ptr<VoxelDataGenerator> &&source,
                 unsigned chunkSize,
                 std::unique_ptr<MapRegionStore> &&mapRegionStore);
    
    // Loads the chunk at the specified region of space.
    // The specified region of space is not required to exactly match the
    // position and size of one of the chunks used internally by the sunlight
    // data grid. This can be an arbitrary region of space with the bounds of
    // the grid.
    // May fault in missing voxels to satisfy the request.
    Array3D<Voxel> load(const AABB &region);
    
    // Edits a single voxel.
    void editSingleVoxel(const glm::vec3 &point, const Voxel &value);
    
    // VoxelData may evict chunks to keep the total chunk count under a limit.
    // Set the limit to the number of chunks needed to represent the region
    // specified in `workingSet'.
    void setWorkingSet(const AABB &workingSet);
    
    // Return the region of voxels which may be accessed during the operation.
    // This is a worst-case estimate of the region of voxel which may be
    // accessed while performing the operation.
    // Knowing this region is useful when determining the region which needs to
    // be locked during the operation.
    AABB getAccessRegionForOperation(TerrainOperation &operation);
    
    AABB getSunlightRegion(AABB sunlightRegion) const;
    
private:
    struct LightNode {
        VoxelDataChunk *chunkPtr;
        glm::ivec3 chunkCellCoords;
        glm::ivec3 voxelCellCoords;
        
        LightNode(VoxelDataChunk *chunkPtr,
                  const glm::ivec3 &chunkCellCoords,
                  const glm::ivec3 &voxelCellCoords)
         : chunkPtr(chunkPtr),
           chunkCellCoords(chunkCellCoords),
           voxelCellCoords(voxelCellCoords)
        {}
    };
    std::shared_ptr<spdlog::logger> _log;
    std::unique_ptr<VoxelDataGenerator> _source;
    PersistentVoxelChunks _chunks;
    
    void propagateSunlight(const GridIndexer &chunkIndexer,
                           const glm::ivec3 &targetColumnCoords);
    
    void seedSunlightInColumn(const GridIndexer &chunkIndexer,
                              const glm::ivec3 &columnCoords,
                              const glm::ivec3 &minSeedCorner,
                              const glm::ivec3 &maxSeedCorner,
                              std::queue<LightNode> &sunlightQueue);
    
    void seedSunlightInTopLayer(VoxelDataChunk *chunkPtr,
                                const glm::ivec3 &chunkCellCoords,
                                const glm::ivec3 &minSeedCorner,
                                const glm::ivec3 &maxSeedCorner,
                                std::queue<LightNode> &sunlightQueue);
    
    void floodNeighbor(VoxelDataChunk *chunkPtr,
                       const glm::ivec3 &chunkCellCoords,
                       const glm::ivec3 &voxelCellCoords,
                       const glm::ivec3 &delta,
                       std::queue<LightNode> &sunlightQueue,
                       bool losslessPropagationOfMaxLight);
    
    std::unique_ptr<VoxelDataChunk> createNewChunk(const AABB &cell, Morton3 index);
};

#endif /* VoxelData_hpp */
