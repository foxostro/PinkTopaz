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
    
    // Accesses to voxel data may require loading surrounding chunks and
    // performing sunlight propagation. This is a conservative estimate of the
    // region where changes may occur. Use this to determine the region which
    // needs to be locked during the access.
    AABB getSunlightRegion(AABB sunlightRegion) const;
    
private:
    // A node in the sunlight propagation BFS queue.
    struct LightNode {
        // A pointer to the chunk that needs to be updated.
        VoxelDataChunk *chunkPtr;
        
        // The cell coordinates of the chunk to be updated.
        glm::ivec3 chunkCellCoords;
        
        // The cell coordinates of the voxel to be updated, specified relative
        // to the chunk.
        glm::ivec3 voxelCellCoords;
        
        // Constructor for convenient creation of a LightNode.
        LightNode(VoxelDataChunk *chunkPtr,
                  const glm::ivec3 &chunkCellCoords,
                  const glm::ivec3 &voxelCellCoords)
         : chunkPtr(chunkPtr),
           chunkCellCoords(chunkCellCoords),
           voxelCellCoords(voxelCellCoords)
        {}
    };
    
    // Logger to use.
    std::shared_ptr<spdlog::logger> _log;
    
    // Procedurally generates voxel data on demand.
    std::unique_ptr<VoxelDataGenerator> _source;
    
    // Helper object for storing and persisting chunks of voxel data.
    // Generally, VoxelData encapsulates the concept of chunks. Clients can
    // remain unaware that voxel data is stored in chunks internally.
    PersistentVoxelChunks _chunks;
    
    // For all chunks in the specified region, perform initial sunlight
    // propagation if it has not yet been done.
    // This will fetch chunks in a neighborhood surrounding the specified region
    // and so callers should ensure appropriate locks have been taken.
    // See also `getSunlightRegion'.
    // region -- The region we should examine.
    void performInitialSunlightPropagationIfNecessary(const AABB &region);
    
    // Propagate sunlight for chunks in the local neighborhood surrounding the
    // target column. When this returns, chunks in the target column will have
    // correct sunlight values. Chunks in neighboring columns may have partial
    // sunlight values which has not yet reached equilibrium.
    // chunkIndexer -- An indexer into the grid of chunks.
    // targetColumnCoords -- The X and Z coordinates of the column to work on.
    //                       The Y coordinate is ignored.
    void propagateSunlight(const GridIndexer &chunkIndexer,
                           const glm::ivec3 &targetColumnCoords);
    
    // Seeds initial sunlight in the specified column, populating the queue.
    // chunkIndexer -- An indexer into the grid of chunks.
    // columnCoords -- The X and Z coordinates of the column to work on.
    //                 The Y coordinate is ignored.
    // minSeedCorner -- When seeding the local neighborhood, we often want to
    //                  avoid seeding the entire column and instead restrict
    //                  ourselves to a subset. This is done to ensure the flood
    //                  fill never leaves the local neighborhood, and never
    //                  leaves the region which is under lock.
    //                  This parameter is the minimum corner of that region.
    //                  The Y coordinate is ignored.
    // maxSeedCorner -- The maximum corner of the region to seed.
    //                  The Y coordinate is ignored.
    // sunlightQueue -- A queue to use when performing the sunlight flood-fill.
    void seedSunlightInColumn(const GridIndexer &chunkIndexer,
                              const glm::ivec3 &columnCoords,
                              const glm::ivec3 &minSeedCorner,
                              const glm::ivec3 &maxSeedCorner,
                              std::queue<LightNode> &sunlightQueue);
    
    // Seeds sunlight in the top layer of the specified chunk.
    // chunkPtr -- A pointer to the chunk to seed.
    // chunkCellCoords -- The cell coordinates of the chunk to seed.
    // minSeedCorner -- When seeding the local neighborhood, we often want to
    //                  avoid seeding the entire column and instead restrict
    //                  ourselves to a subset. This is done to ensure the flood
    //                  fill never leaves the local neighborhood, and never
    //                  leaves the region which is under lock.
    //                  This parameter is the minimum corner of that region.
    //                  The Y coordinate is ignored.
    // maxSeedCorner -- The maximum corner of the region to seed.
    //                  The Y coordinate is ignored.
    // sunlightQueue -- A queue to use when performing the sunlight flood-fill.
    void seedSunlightInTopLayer(VoxelDataChunk *chunkPtr,
                                const glm::ivec3 &chunkCellCoords,
                                const glm::ivec3 &minSeedCorner,
                                const glm::ivec3 &maxSeedCorner,
                                std::queue<LightNode> &sunlightQueue);
    
    // Performs one step of the sunlight propagation flood fill.
    // chunkPtr -- A pointer to the chunk we're working on.
    // chunkCellCoords -- The cell coordinates of the chunk we're working on.
    // voxelCellCoords -- The cell coordinates of the voxel we're working on,
    //                    specified relative to the chunk.
    // delta -- An integer offset from the voxelCellCoords to the cell coords of
    //          the voxel we're flooding into. This should be a step of size 1.
    // sunlightQueue -- The flood-fill BFS queue.
    // losslessPropagationOfMaxLight -- When flooding in certain directions,
    //                                  (i.e. down) we may want to avoid having
    //                                  the light level decrease by one.
    void floodNeighbor(VoxelDataChunk *chunkPtr,
                       const glm::ivec3 &chunkCellCoords,
                       const glm::ivec3 &voxelCellCoords,
                       const glm::ivec3 &delta,
                       std::queue<LightNode> &sunlightQueue,
                       bool losslessPropagationOfMaxLight);
    
    // Returns a new chunk for the specified cell.
    // cell -- The bounding box of the chunk.
    // index -- An index into the chunk grid corresponding to the cell.
    std::unique_ptr<VoxelDataChunk> createNewChunk(const AABB &cell, Morton3 index);
};

#endif /* VoxelData_hpp */
