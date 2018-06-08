//
//  VoxelData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#include "Terrain/VoxelData.hpp"
#include "Terrain/TerrainConfig.hpp"
#include "Grid/GridIndexerRange.hpp"

using namespace glm;

// This assert macro is used for invariants that we cannot test because it
// degrades performance too much in a Debug build. Mostly for documentation.
#define VERBOSE_ASSERT(...)

VoxelData::VoxelData(std::shared_ptr<spdlog::logger> log,
                     std::unique_ptr<VoxelDataGenerator> &&source,
                     unsigned chunkSize,
                     std::unique_ptr<MapRegionStore> &&mapRegionStore,
                     const std::shared_ptr<TaskDispatcher> &dispatcher)
: GridIndexer(source->boundingBox(), source->gridResolution()),
  _log(log),
  _source(std::move(source)),
  _chunks(log,
          _source->boundingBox(),
          _source->gridResolution(),
          chunkSize,
          std::move(mapRegionStore),
          [=](const AABB &cell, Morton3 index){
              return createNewChunk(cell, index);
          }),
  _dispatcher(dispatcher)
{}

void VoxelData::propagateSunlight(const GridIndexer &chunkIndexer,
                                  const ivec3 &targetColumnCoords)
{
    std::queue<LightNode> sunlightQueue;
    
    // Seed sunlight in all columns of the local neighborhood.
    constexpr size_t numNeighborhoodColumns = 9;
    constexpr int b = TERRAIN_CHUNK_SIZE;
    constexpr int a = MAX_LIGHT;
    constexpr int y = 0;
    const std::array<ivec3, numNeighborhoodColumns> neighborhoodColumn = {{
        ivec3(targetColumnCoords.x - 1, y, targetColumnCoords.z - 1), // (-1, -1)
        ivec3(targetColumnCoords.x - 0, y, targetColumnCoords.z - 1), // ( 0, -1)
        ivec3(targetColumnCoords.x + 1, y, targetColumnCoords.z - 1), // (+1, -1)
        ivec3(targetColumnCoords.x - 1, y, targetColumnCoords.z - 0), // (-1,  0)
        ivec3(targetColumnCoords.x - 0, y, targetColumnCoords.z - 0), // ( 0,  0)
        ivec3(targetColumnCoords.x + 1, y, targetColumnCoords.z - 0), // (+1,  0)
        ivec3(targetColumnCoords.x - 1, y, targetColumnCoords.z + 1), // (-1, +1)
        ivec3(targetColumnCoords.x - 0, y, targetColumnCoords.z + 1), // ( 0, +1)
        ivec3(targetColumnCoords.x + 1, y, targetColumnCoords.z + 1), // (+1, +1)
    }};
    const std::array<ivec3, numNeighborhoodColumns> minSeedCorner = {{
        ivec3(b - a, y, b - a), // (-1, -1)
        ivec3(    0, y, b - a), // ( 0, -1)
        ivec3(    0, y, b - a), // (+1, -1)
        ivec3(b - a, y,     0), // (-1,  0)
        ivec3(    0, y,     0), // ( 0,  0)
        ivec3(    0, y,     0), // (+1,  0)
        ivec3(b - a, y,     0), // (-1, +1)
        ivec3(    0, y,     0), // ( 0, +1)
        ivec3(    0, y,     0), // (+1, +1)
    }};
    const std::array<ivec3, numNeighborhoodColumns> maxSeedCorner = {{
        ivec3(b, y, b), // (-1, -1)
        ivec3(b, y, b), // ( 0, -1)
        ivec3(a, y, b), // (+1, -1)
        ivec3(b, y, b), // (-1,  0)
        ivec3(b, y, b), // ( 0,  0)
        ivec3(a, y, b), // (+1,  0)
        ivec3(b, y, a), // (-1, +1)
        ivec3(b, y, a), // ( 0, +1)
        ivec3(a, y, a), // (+1, +1)
    }};
    for (size_t i = 0; i < numNeighborhoodColumns; ++i) {
        seedSunlightInColumn(chunkIndexer,
                             neighborhoodColumn[i],
                             minSeedCorner[i],
                             maxSeedCorner[i],
                             sunlightQueue);
    }

    // Perform the flood fill using a breadth-first iteration of voxels.
    while (!sunlightQueue.empty()) {
        const LightNode &node = sunlightQueue.front();
        const auto chunk = node.chunkPtr;
        const ivec3 chunkCoords = node.chunkCellCoords;
        const ivec3 voxelCoords = node.voxelCellCoords;
        sunlightQueue.pop();
        
        floodNeighbor(chunk, chunkCoords, voxelCoords, ivec3(-1,  0,  0), sunlightQueue, false);
        floodNeighbor(chunk, chunkCoords, voxelCoords, ivec3(+1,  0,  0), sunlightQueue, false);
        floodNeighbor(chunk, chunkCoords, voxelCoords, ivec3( 0,  0, -1), sunlightQueue, false);
        floodNeighbor(chunk, chunkCoords, voxelCoords, ivec3( 0,  0, +1), sunlightQueue, false);
        floodNeighbor(chunk, chunkCoords, voxelCoords, ivec3( 0, -1,  0), sunlightQueue, true);
    }
}

void VoxelData::seedSunlightInColumn(const GridIndexer &chunkIndexer,
                                     const ivec3 &columnCoords,
                                     const ivec3 &minSeedCorner,
                                     const ivec3 &maxSeedCorner,
                                     std::queue<LightNode> &sunlightQueue)
{
    const ivec3 &res = chunkIndexer.gridResolution();
    
    // Skip columns that are outside the bounds of the voxel grid.
    // This can occur when propagating sunlight at the edge of the world.
    if (columnCoords.x < 0 || columnCoords.x >= res.x ||
        columnCoords.z < 0 || columnCoords.z >= res.z) {
        return;
    }
        
    // Walk down each column until we reach the first non-Sky chunk.
    // Seed sunlight at the top of this chunk and then move on to the next
    // column.
    for (ivec3 cellCoords{columnCoords.x, res.y-1, columnCoords.z}; cellCoords.y >= 0; --cellCoords.y) {
        AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
        Morton3 chunkIndex(cellCoords);
        auto chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex).get();
        assert(chunkPtr);
        if (chunkPtr->getType() == VoxelDataChunk::Sky) {
            continue;
        } else {
            seedSunlightInTopLayer(chunkPtr, cellCoords,
                                   minSeedCorner, maxSeedCorner,
                                   sunlightQueue);
            break;
        }
    } // for column y
}

void VoxelData::seedSunlightInTopLayer(VoxelDataChunk *chunkPtr,
                                       const ivec3 &chunkCellCoords,
                                       const ivec3 &minSeedCorner,
                                       const ivec3 &maxSeedCorner,
                                       std::queue<LightNode> &sunlightQueue)
{
    VERBOSE_ASSERT(chunkPtr);
    
    const ivec3 &res = chunkPtr->gridResolution();
    
    VERBOSE_ASSERT(minSeedCorner.x >= 0 && minSeedCorner.x < res.x);
    VERBOSE_ASSERT(minSeedCorner.z >= 0 && minSeedCorner.z < res.z);
    VERBOSE_ASSERT(maxSeedCorner.x >= 0 && maxSeedCorner.x <= res.x);
    VERBOSE_ASSERT(maxSeedCorner.z >= 0 && maxSeedCorner.z <= res.z);
    VERBOSE_ASSERT(maxSeedCorner.x > minSeedCorner.x);
    VERBOSE_ASSERT(maxSeedCorner.z > minSeedCorner.z);
    
    for (ivec3 cellCoords{minSeedCorner.x, res.y-1, minSeedCorner.z}; cellCoords.x < maxSeedCorner.x; ++cellCoords.x) {
        for (cellCoords.z = minSeedCorner.z; cellCoords.z < maxSeedCorner.z; ++cellCoords.z) {
            Voxel voxel = chunkPtr->get(cellCoords);
            if (voxel.value == 0) {
                voxel.sunLight = MAX_LIGHT;
                chunkPtr->set(cellCoords, voxel);
                sunlightQueue.emplace(LightNode(chunkPtr,
                                                chunkCellCoords,
                                                cellCoords));
            }
        }
    }
}

void VoxelData::floodNeighbor(VoxelDataChunk *chunkPtr,
                              const ivec3 &chunkCellCoords,
                              const ivec3 &voxelCellCoords,
                              const ivec3 &delta,
                              std::queue<LightNode> &sunlightQueue,
                              bool losslessPropagationOfMaxLight)
{
    VERBOSE_ASSERT(chunkPtr);
    
    const Voxel nodeVoxel = chunkPtr->get(voxelCellCoords);
    VERBOSE_ASSERT(nodeVoxel.value == 0);
    
    const ivec3 &chunkSize = chunkPtr->gridResolution();
    
    ivec3 neighborCellCoords = voxelCellCoords + delta;
    ivec3 neighborChunkCellCoords = chunkCellCoords;
    
    // Adjust the neighbor coords if we're crossing a chunk boundary right now.
    if (neighborCellCoords.x < 0) {
        neighborCellCoords.x += chunkSize.x;
        neighborChunkCellCoords.x--;
    } else if (neighborCellCoords.y < 0) {
        neighborCellCoords.y += chunkSize.y;
        neighborChunkCellCoords.y--;
    } else if (neighborCellCoords.z < 0) {
        neighborCellCoords.z += chunkSize.z;
        neighborChunkCellCoords.z--;
    } else if (neighborCellCoords.x >= chunkSize.x) {
        neighborCellCoords.x -= chunkSize.x;
        neighborChunkCellCoords.x++;
    } else if (neighborCellCoords.y >= chunkSize.y) {
        neighborCellCoords.y -= chunkSize.y;
        neighborChunkCellCoords.y++;
    } else if (neighborCellCoords.z >= chunkSize.z) {
        neighborCellCoords.z -= chunkSize.z;
        neighborChunkCellCoords.z++;
    }
    
    // Get the neighbor chunk. We only have to consult `_chunks' if we're
    // crossing a chunk boundary right now.
    VoxelDataChunk *neighborChunk = nullptr;
    if (neighborChunkCellCoords == chunkCellCoords) {
        neighborChunk = chunkPtr;
    } else {
        // Note that we seed sunlight in such a way that the flood fill will
        // never reach outside the local neighborhood.
        const GridIndexer &indexer = _chunks.getChunkIndexer();
        if (indexer.inbounds(neighborChunkCellCoords)) {
            // We don't allow the floodfill to reach outside the voxel grid.
            // This can happen at the edge of the world.
            const AABB neighborChunkBoundingBox = indexer.cellAtCellCoords(neighborChunkCellCoords);
            neighborChunk = _chunks.get(neighborChunkBoundingBox, Morton3(neighborChunkCellCoords)).get();
        }
    }
    if (!neighborChunk) {
        // The neighbor is out of bounds and there's nothing left to do.
        return;
    }
    
    Voxel neighborVoxel = neighborChunk->get(neighborCellCoords);
    
    int nodeValue = nodeVoxel.value;
    int neighborValue = neighborVoxel.value;
    
    int nodeSunLight = nodeVoxel.sunLight;
    int neighborSunLight = neighborVoxel.sunLight;
    
    if ((neighborValue == 0) && (nodeValue == 0)) {
        bool didModifyNeighborVoxel = false;
        if (losslessPropagationOfMaxLight && nodeSunLight == MAX_LIGHT) {
            neighborVoxel.sunLight = MAX_LIGHT;
            didModifyNeighborVoxel = true;
        } else if ((neighborSunLight + 2) <= nodeSunLight) {
            neighborSunLight = nodeSunLight - 1;
            assert(neighborSunLight >= 0 && neighborSunLight <= MAX_LIGHT);
            neighborVoxel.sunLight = neighborSunLight;
            didModifyNeighborVoxel = true;
        }
        if (didModifyNeighborVoxel) {
            neighborChunk->set(neighborCellCoords, neighborVoxel);
            sunlightQueue.emplace(LightNode(neighborChunk,
                                            neighborChunkCellCoords,
                                            neighborCellCoords));
        }
    }
}

void VoxelData::performInitialSunlightPropagationIfNecessary(const AABB &region)
{
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    const glm::ivec3 &res = chunkIndexer.gridResolution();
    const ivec3 minChunkCoords = chunkIndexer.cellCoordsAtPoint(region.mins());
    const ivec3 maxChunkCoords = chunkIndexer.cellCoordsAtPointRoundUp(region.maxs());
    
    auto iterateColumns = [&](auto fn){
        for (ivec3 chunkCoords{minChunkCoords.x, 0, minChunkCoords.z}; chunkCoords.x < maxChunkCoords.x; ++chunkCoords.x) {
            for (chunkCoords.z = minChunkCoords.z; chunkCoords.z < maxChunkCoords.z; ++chunkCoords.z) {
                fn(chunkCoords);
            }
        }
    };
    
    // Determine which columns are missing.
    std::unordered_set<Morton3> missingChunks;
    iterateColumns([&](ivec3 chunkCoords){
        for (chunkCoords.y = 0; chunkCoords.y < res.y; ++chunkCoords.y) {
            Morton3 chunkIndex(chunkCoords);
            auto maybeChunkPtr = _chunks.getIfExists(chunkIndex);
            if (!maybeChunkPtr) {
                missingChunks.insert(chunkIndex);
            }
        }
    });
    
    // Fetch missing chunks.
    auto futuresFetchChunks = _dispatcher->map(missingChunks, [&, this](const Morton3 &chunkIndex){
        ivec3 chunkCoords = chunkIndex.decode();
        AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(chunkCoords);
        (void)_chunks.get(chunkBoundingBox, chunkIndex);
    });
    waitForAll(futuresFetchChunks);
    
    // For each column, propagate sunlight if the columns is incomplete.
    // TODO: We can improve on this by seeding all columns at once and running through the BFS queue one time.
    bool modifiedAnyChunk = false;
    auto processColumn = [&](ivec3 chunkCoords){
        // Have all chunks in the column already undergone initial propagation?
        bool columnIsComplete = true;
        for (chunkCoords.y = 0; chunkCoords.y < res.y; ++chunkCoords.y) {
            Morton3 chunkIndex(chunkCoords);
            auto maybeChunkPtr = _chunks.getIfExists(chunkIndex);
            auto &chunkPtr = maybeChunkPtr.value();
            if (!chunkPtr->complete) {
                columnIsComplete = false;
                break;
            }
        }
        
        if (!columnIsComplete) {
            propagateSunlight(chunkIndexer, chunkCoords);
            modifiedAnyChunk = true;
            
            // Note that we'll want to save these changes back to disk later.
            for (chunkCoords.y = 0; chunkCoords.y < res.y; ++chunkCoords.y) {
                Morton3 chunkIndex(chunkCoords);
                auto maybeChunkPtr = _chunks.getIfExists(chunkIndex);
                maybeChunkPtr.value()->complete = true;
            }
        }
    };
    
    // When we propagate sunlight for the center column, there is a region of
    // space in neighboring columns which is correct. Though, not the entire
    // column will be correct. If the requested region only touches this safe
    // region then we can propagate sunlight for the center column of the
    // neighborhood only.
    
    AABB safeRegion;
    {
        safeRegion = _chunks.getChunkIndexer().cellAtPoint(region.center);
        
        safeRegion = safeRegion.inset(-vec3((float)TERRAIN_CHUNK_SIZE-MAX_LIGHT, 0, (float)TERRAIN_CHUNK_SIZE-MAX_LIGHT));
        
        vec3 mins = safeRegion.mins();
        mins.y = boundingBox().mins().y;
        
        vec3 maxs = safeRegion.maxs();
        maxs.y = boundingBox().maxs().y;
        
        safeRegion.center = (maxs + mins) * 0.5f;
        safeRegion.extent = (maxs - mins) * 0.5f;
    }
    
    bool useFastPath = (safeRegion.intersect(region) == region);
    
    if (useFastPath) {
        const ivec3 chunkCoords = chunkIndexer.cellCoordsAtPoint(region.center);
        processColumn(chunkCoords);
    } else {
        iterateColumns(processColumn);
    }
    
    // Save changes back to disk.
    if (modifiedAnyChunk) {
        std::vector<Future<void>> futures;
        iterateColumns([&](ivec3 chunkCoords){
            for (chunkCoords.y = 0; chunkCoords.y < res.y; ++chunkCoords.y) {
                Morton3 chunkIndex(chunkCoords);
                futures.emplace_back(_dispatcher->async([this, chunkIndex](){
                    _chunks.store(chunkIndex);
                }));
            }
        });
        waitForAll(futures);
    }
}

Array3D<Voxel> VoxelData::load(const AABB &region)
{
    performInitialSunlightPropagationIfNecessary(region);
    return _chunks.loadSubRegion(region);
}

void VoxelData::editSingleVoxel(const vec3 &point, const Voxel &value)
{
    // TODO: Update lighting in the region surrounding the point and return the region where voxels actually did change.
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    const AABB chunkBoundingBox = chunkIndexer.cellAtPoint(point);
    const Morton3 chunkIndex = chunkIndexer.indexAtPoint(point);
    auto chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex);
    chunkPtr->set(point, value);
    _chunks.store(chunkIndex);
}

AABB VoxelData::getAccessRegionForOperation(TerrainOperation &operation)
{
    AABB region = _source->snapRegionToCellBoundaries(operation.getAffectedRegion());
    region = _chunks.getChunkIndexer().snapRegionToCellBoundaries(region);
    return region;
}

std::unique_ptr<VoxelDataChunk>
VoxelData::createNewChunk(const AABB &cell, Morton3 chunkIndex)
{
    if (cell.center.y > 64.f || cell.center.y < 0.f) {
        const auto adjusted = _source->snapRegionToCellBoundaries(cell);
        const auto res = _source->countCellsInRegion(adjusted);
        if (cell.center.y < 0.f) {
            return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createGroundChunk(adjusted, res));
        } else {
            return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createSkyChunk(adjusted, res));
        }
    } else {
        return std::make_unique<VoxelDataChunk>(VoxelDataChunk::createArrayChunk(_source->copy(cell)));
    }
}

AABB VoxelData::getSunlightRegion(AABB sunlightRegion) const
{
    sunlightRegion = _chunks.getChunkIndexer().snapRegionToCellBoundaries(sunlightRegion);
    
    sunlightRegion = sunlightRegion.inset(-vec3((float)TERRAIN_CHUNK_SIZE, 0, (float)TERRAIN_CHUNK_SIZE));
    
    vec3 mins = sunlightRegion.mins();
    mins.y = boundingBox().mins().y;
    
    vec3 maxs = sunlightRegion.maxs();
    maxs.y = boundingBox().maxs().y;
    
    sunlightRegion.center = (maxs + mins) * 0.5f;
    sunlightRegion.extent = (maxs - mins) * 0.5f;
    
    return sunlightRegion;
}
