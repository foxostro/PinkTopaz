//
//  SunlightData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#include "Terrain/SunlightData.hpp"
#include "Terrain/TerrainConfig.hpp"
#include "Grid/GridIndexerRange.hpp"

using namespace glm;

// This assert macro is used for invariants that we cannot test because it
// degrades performance too much in a Debug build. Mostly for documentation.
#define VERBOSE_ASSERT(...)

SunlightData::SunlightData(std::shared_ptr<spdlog::logger> log,
                           std::unique_ptr<VoxelData> &&source,
                           unsigned chunkSize,
                           std::unique_ptr<MapRegionStore> &&mapRegionStore)
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
          })
{
    _log->info("SunlightData -- Beginning generation of sunlight data.");
    const auto startTime = std::chrono::steady_clock::now();
    
    _log->info("SunlightData -- Performing sunlight floodfill propagation.");
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    const ivec3 res = chunkIndexer.gridResolution();
    for (ivec3 columnCoords{0, 0, 0}; columnCoords.x < res.x; ++columnCoords.x) {
        for (columnCoords.z = 0; columnCoords.z < res.z; ++columnCoords.z) {
            propagateSunlight(chunkIndexer, columnCoords);
        }
    }
    
    const auto duration = std::chrono::steady_clock::now() - startTime;
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    _log->info("SunlightData -- Finished sunlight propagation. (took {} ms)",
               std::to_string(ms.count()));
}

void SunlightData::propagateSunlight(const GridIndexer &chunkIndexer,
                                     const ivec3 &targetColumnCoords)
{
    std::queue<LightNode> sunlightQueue;
    
    const ivec3 &res = chunkIndexer.gridResolution();
    
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
    
    // Mark chunks in the target column as complete.
    for (ivec3 cellCoords{targetColumnCoords.x, res.y-1, targetColumnCoords.x}; cellCoords.y >= 0; --cellCoords.y) {
        AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
        Morton3 chunkIndex(cellCoords);
        auto chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex).get();
        assert(chunkPtr);
        chunkPtr->complete = true;
    }
}

void SunlightData::seedSunlightInColumn(const GridIndexer &chunkIndexer,
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

void SunlightData::seedSunlightInTopLayer(VoxelDataChunk *chunkPtr,
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

void SunlightData::floodNeighbor(VoxelDataChunk *chunkPtr,
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

#if 0
bool SunlightData::isChunkComplete(const vec3 &point)
{
    Morton3 index = _chunks.getChunkIndexer().indexAtPoint(point);
    auto maybeChunk = _chunks.getIfExists(index);
    if (maybeChunk) {
        std::shared_ptr<VoxelDataChunk> chunk = *maybeChunk;
        assert(chunk);
        if (chunk->complete) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<VoxelDataChunk> SunlightData::chunkAtCellCoords(const ivec3 &cellCoords)
{
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    const AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
    const Morton3 index(cellCoords);
    auto chunkPtr = _chunks.get(chunkBoundingBox, index);
    return chunkPtr;
}
#endif

Array3D<Voxel> SunlightData::load(const AABB &region)
{
#if 0
    bool chunkIsComplete = isChunkComplete(region.center);
    
    if (!chunkIsComplete) {
        std::queue<LightNode> sunlightQueue;
        
        const AABB sunlightRegion = getSunlightRegion(region);
        const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
        const ivec3 minCellCoords = chunkIndexer.cellCoordsAtPoint(sunlightRegion.mins());
        const ivec3 maxCellCoords = chunkIndexer.cellCoordsAtPointRoundUp(sunlightRegion.maxs());
        
        for (ivec3 cellCoords = minCellCoords; cellCoords.x < maxCellCoords.x; ++cellCoords.x) {
            for (cellCoords.z = minCellCoords.z; cellCoords.z < maxCellCoords.z; ++cellCoords.z) {
                for (cellCoords.y = maxCellCoords.y-1; cellCoords.y >= minCellCoords.y; --cellCoords.y) {
                    auto chunkPtr = chunkAtCellCoords(cellCoords);
                    assert(chunkPtr);
                    seedSunlightInTopLayer(*chunkPtr, sunlightQueue);
                }
            }
        }
        
        while (!sunlightQueue.empty()) {
            const glm::vec3 voxelPos = sunlightQueue.front();
            sunlightQueue.pop();
            //            floodNeighbor(voxelPos, vec3(-1,  0,  0), sunlightQueue, false);
            //            floodNeighbor(voxelPos, vec3(+1,  0,  0), sunlightQueue, false);
            //            floodNeighbor(voxelPos, vec3( 0,  0, -1), sunlightQueue, false);
            //            floodNeighbor(voxelPos, vec3( 0,  0, +1), sunlightQueue, false);
            floodNeighbor(voxelPos, vec3( 0, -1,  0), sunlightQueue, true);
        }
        
        ivec3 cellCoords = chunkIndexer.cellCoordsAtPoint(region.center);
        for (cellCoords.y = maxCellCoords.y-1; cellCoords.y >= minCellCoords.y; --cellCoords.y) {
            auto chunkPtr = chunkAtCellCoords(cellCoords);
            assert(chunkPtr);
            chunkPtr->complete = true;
        }
    } // if (!chunkIsComplete)
#endif
    
    return _chunks.loadSubRegion(region);
}

void SunlightData::editSingleVoxel(const vec3 &point, const Voxel &value)
{
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    AABB actualAreaOfSunlightChange = _source->cellAtPoint(point);
    bool done = false;
    vec3 chunkPoint = _chunks.cellCenterAtPoint(point);
    
    while(!done) {
        const AABB chunkBoundingBox = chunkIndexer.cellAtPoint(chunkPoint);
        VoxelDataChunk chunk = _chunks.load(chunkBoundingBox);
        
        if (chunk.getType() == VoxelDataChunk::Ground) {
            done = true;
        } else if (chunk.getType() == VoxelDataChunk::Sky) {
            // We'll convert this chunk to an array type chunk and then move on.
            chunk.convertToArray();
        } else {
            for (ivec3 cellCoords = chunk.cellCoordsAtPoint(point); cellCoords.y >= 0; --cellCoords.y) {
                const Voxel &voxel = chunk.get(cellCoords);
                if (voxel.value == 0) {
                    const AABB voxelCell = chunk.cellAtCellCoords(cellCoords);
                    actualAreaOfSunlightChange = actualAreaOfSunlightChange.unionBox(voxelCell);
                } else {
                    done = true;
                    break;
                }
            }
        }
        
        chunkPoint.y -= 2.f * chunkBoundingBox.extent.y;
    }
    
    _chunks.invalidate(actualAreaOfSunlightChange);
    
    _source->editSingleVoxel(point, value);
}

void SunlightData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
    _chunks.setWorkingSet(workingSet);
}

AABB SunlightData::getAccessRegionForOperation(TerrainOperation &operation)
{
    return getSunlightRegion(operation.getAffectedRegion());
}

std::unique_ptr<VoxelDataChunk>
SunlightData::createNewChunk(const AABB &cell, Morton3 chunkIndex)
{
    VoxelDataChunk chunks = _source->load(cell);
    chunks.complete = (chunks.getType() != VoxelDataChunk::Array);
    return std::make_unique<VoxelDataChunk>(std::move(chunks));
}

AABB SunlightData::getSunlightRegion(AABB sunlightRegion) const
{
    sunlightRegion = _chunks.getChunkIndexer().snapRegionToCellBoundaries(sunlightRegion);
    
    sunlightRegion = sunlightRegion.inset(vec3((float)TERRAIN_CHUNK_SIZE, 0, (float)TERRAIN_CHUNK_SIZE));
    
    vec3 mins = sunlightRegion.mins();
    mins.y = boundingBox().mins().y;
    
    vec3 maxs = sunlightRegion.maxs();
    maxs.y = boundingBox().maxs().y;
    
    sunlightRegion.center = (maxs + mins) * 0.5f;
    sunlightRegion.extent = (maxs - mins) * 0.5f;
    
    return sunlightRegion;
}
