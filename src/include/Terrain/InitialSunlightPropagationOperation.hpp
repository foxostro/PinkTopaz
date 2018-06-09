//
//  InitialSunlightPropagationOperation.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/8/18.
//
//

#ifndef InitialSunlightPropagationOperation_hpp
#define InitialSunlightPropagationOperation_hpp

#include "Terrain/PersistentVoxelChunks.hpp"
#include "Terrain/TerrainOperation.hpp"
#include "Grid/ConcurrentSparseGrid.hpp"
#include "TaskDispatcher.hpp"

#include <spdlog/spdlog.h>
#include <queue>

// It's not clear that cached chunk access is actually beneficial.
#define CACHED_CHUNK_ACCESS 0

// Propagates sunlight through newly created voxel data chunks.
class InitialSunlightPropagationOperation
{
public:
    // No default constructor.
    InitialSunlightPropagationOperation() = delete;
    
    // Constructor.
    // log -- The logger to use.
    // persistentVoxelChunks -- The peristent chunks object fetches voxel data
    //                          chunks for us from memory, file, or from the
    //                          generator.
    // dispatcher -- Dispatcher used to fetch voxels data from the generator.
    //               This permits parallel fetch and generation of voxel data.
    InitialSunlightPropagationOperation(std::shared_ptr<spdlog::logger> log,
                                        PersistentVoxelChunks &persistentVoxelChunks,
                                        const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // For all chunks in the specified region, perform initial sunlight
    // propagation if it has not yet been done.
    // This will fetch chunks in a neighborhood surrounding the specified region
    // and so callers should ensure appropriate locks have been taken.
    // See also `getSunlightRegion'.
    // region -- The region we should examine.
    void performInitialSunlightPropagationIfNecessary(const AABB &region);
    
private:
    // Adapt the PersistentVoxelChunks object to provide more convenient API.
    class ChunksAdapter
    {
    public:
        ChunksAdapter(PersistentVoxelChunks &persistentVoxelChunks)
         : _persistentVoxelChunks(persistentVoxelChunks)
#if CACHED_CHUNK_ACCESS
           ,_cache(persistentVoxelChunks.getChunkIndexer().boundingBox(),
                  persistentVoxelChunks.getChunkIndexer().gridResolution())
#endif
        {}
        
        // Re-saves the chunk for the specified index.
        // This is useful when a chunk is retrieved via get() and then modified.
        inline void store(Morton3 index, const std::shared_ptr<TaskDispatcher> &dispatcher)
        {
            _persistentVoxelChunks.store(index, dispatcher);
        }
        
        // Returns the chunk, creating it if necessary, but prefering to fetch it
        // from the map region file.
        // index -- A unique index to identify the chunk in the sparse grid.
        VoxelDataChunk* get(Morton3 index)
        {
#if CACHED_CHUNK_ACCESS
            return _cache.get(index, [&]{
                const AABB boundingBox = _cache.cellAtCellCoords(index.decode());
                std::shared_ptr<VoxelDataChunk> smartPointerChunk = _persistentVoxelChunks.get(boundingBox, index);
                VoxelDataChunk *pointerChunk = smartPointerChunk.get();
                return pointerChunk;
            });
#else
            const AABB boundingBox = getChunkIndexer().cellAtCellCoords(index.decode());
            std::shared_ptr<VoxelDataChunk> smartPointerChunk = _persistentVoxelChunks.get(boundingBox, index);
            VoxelDataChunk *pointerChunk = smartPointerChunk.get();
            return pointerChunk;
#endif
        }
        
        // Returns true if the specified chunk is missing from the grid.
        // In this case, the chunk is not present in either the cache or in the
        // backing store.
        bool isMissing(Morton3 index)
        {
#if CACHED_CHUNK_ACCESS
            bool missing = false;
            if (!_cache.get(index)) {
                boost::optional<std::shared_ptr<VoxelDataChunk>> maybeChunkPtr = _persistentVoxelChunks.getIfExists(index);
                if (maybeChunkPtr) {
                    VoxelDataChunk *rawPointer = maybeChunkPtr->get();
                    _cache.set(index, rawPointer);
                } else {
                    missing = true;
                }
            }
            return missing;
#else
            boost::optional<std::shared_ptr<VoxelDataChunk>> maybeChunkPtr = _persistentVoxelChunks.getIfExists(index);
            if (maybeChunkPtr) {
                return false;
            } else {
                return true;
            }
#endif
        }
        
        // Return an indexer for the grid of chunks.
        inline const GridIndexer& getChunkIndexer() const
        {
#if CACHED_CHUNK_ACCESS
            return _cache;
#else
            return _persistentVoxelChunks.getChunkIndexer();
#endif
        }
        
    private:
        // Backing data store for voxel data chunks.
        PersistentVoxelChunks &_persistentVoxelChunks;
        
#if CACHED_CHUNK_ACCESS
        // Caches pointers to chunks retrieved from the backing data store.
        ConcurrentSparseGrid<VoxelDataChunk*> _cache;
#endif
    };
    
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
    
    // Provides access to the voxel chunks backing store.
    ChunksAdapter _chunks;
    
    // Dispatcher used to fetch voxels data from the generator. We do this
    // to provide the opportunity for the voxel data generator to generate the
    // chunks in parallel.
    std::shared_ptr<TaskDispatcher> _dispatcher;
    
    // Propagate sunlight for chunks in the local neighborhood surrounding the
    // target column. When this returns, chunks in the target column will have
    // correct sunlight values. Chunks in neighboring columns may have partial
    // sunlight values which has not yet reached equilibrium.
    // targetColumnCoords -- The X and Z coordinates of the column to work on.
    //                       The Y coordinate is ignored.
    void propagateSunlight(const glm::ivec3 &targetColumnCoords);
    
    // Seeds initial sunlight in the specified column, populating the queue.
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
    void seedSunlightInColumn(const glm::ivec3 &columnCoords,
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
};

#endif /* InitialSunlightPropagationOperation_hpp */
