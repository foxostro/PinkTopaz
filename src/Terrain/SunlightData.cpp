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

template<typename Function>
void iterateHorizontalSlice(const ivec3 &res, Function &&fn){
    for (ivec3 cellCoords{0,0,0}; cellCoords.x < res.x; ++cellCoords.x) {
        for (cellCoords.z = 0; cellCoords.z < res.z; ++cellCoords.z) {
            fn(cellCoords);
        }
    }
}

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
    
    std::queue<LightNode> sunlightQueue;
    const GridIndexer &chunkIndexer = _chunks.getChunkIndexer();
    const ivec3 res = chunkIndexer.gridResolution();
    
    auto iterateColumns = [&](auto fn) {
        for (ivec3 cellCoords{0, res.y-1, 0}; cellCoords.x < res.x; ++cellCoords.x) {
            for (cellCoords.z = 0; cellCoords.z < res.z; ++cellCoords.z) {
                fn(cellCoords);
            }
        }
    };
    
    auto iterateChunks = [&](auto fn) {
        iterateColumns([&](ivec3 cellCoords){
            for (cellCoords.y = res.y-1; cellCoords.y >= 0; --cellCoords.y) {
                fn(cellCoords);
            }
        });
    };
    
    _log->info("SunlightData -- Fetching voxels for all chunks in the world.");
    iterateChunks([&](const ivec3 &cellCoords){
        AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
        Morton3 chunkIndex(cellCoords);
        (void)_chunks.get(chunkBoundingBox, chunkIndex);
        _log->info("SunlightData -- Fetched chunk at {}", chunkBoundingBox);
    });
    
    _log->info("SunlightData -- Seeding sunlight for all chunks along the ceiling of the world.");
    iterateColumns([&](ivec3 cellCoords){
        // Walk down each column until we reach the first non-Sky chunk.
        // Seed sunlight at the top of this chunk and then move on to the next
        // column.
        for (cellCoords.y = res.y-1; cellCoords.y >= 0; --cellCoords.y) {
            AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
            Morton3 chunkIndex(cellCoords);
            auto chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex);
            assert(chunkPtr);
            if (chunkPtr->getType() == VoxelDataChunk::Sky) {
                continue;
            } else {
                seedSunlightInTopLayer(chunkPtr, cellCoords, sunlightQueue);
                _log->info("SunlightData -- Seeded sunlight for column "\
                           "<{}, {}>", cellCoords.x, cellCoords.z);
                break;
            }
        }
    });
    
    _log->info("SunlightData -- Performing sunlight floodfill propagation.");
    _log->info("SunlightData -- sunlightQueue contains {} items.", sunlightQueue.size());
    const auto startTime = std::chrono::steady_clock::now();
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
    
    const auto duration = std::chrono::steady_clock::now() - startTime;
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    _log->info("SunlightData -- Finished generation of sunlight data."\
               "(took {} ms)",
               std::to_string(ms.count()));
}

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

void SunlightData::seedSunlightInTopLayer(const std::shared_ptr<VoxelDataChunk> &chunkPtr,
                                          const ivec3 &chunkCellCoords,
                                          std::queue<LightNode> &sunlightQueue)
{
    assert(chunkPtr);
    const ivec3 &res = chunkPtr->gridResolution();
    for (ivec3 cellCoords{0, res.y-1, 0}; cellCoords.x < res.x; ++cellCoords.x) {
        for (cellCoords.z = 0; cellCoords.z < res.z; ++cellCoords.z) {
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

void SunlightData::floodNeighbor(const std::shared_ptr<VoxelDataChunk> &chunkPtr,
                                 const ivec3 &chunkCellCoords,
                                 const ivec3 &voxelCellCoords,
                                 const ivec3 &delta,
                                 std::queue<LightNode> &sunlightQueue,
                                 bool losslessPropagationOfMaxLight)
{
    const Voxel nodeVoxel = chunkPtr->get(voxelCellCoords);
    assert(nodeVoxel.value == 0);
    
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
    std::shared_ptr<VoxelDataChunk> neighborChunk;
    if (neighborChunkCellCoords != chunkCellCoords) {
        const GridIndexer &indexer = _chunks.getChunkIndexer();
        if (indexer.inbounds(neighborChunkCellCoords)) {
            const AABB neighborChunkBoundingBox = indexer.cellAtCellCoords(neighborChunkCellCoords);
            neighborChunk = _chunks.get(neighborChunkBoundingBox, Morton3(neighborChunkCellCoords));
        }
    } else {
        neighborChunk = chunkPtr;
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

Array3D<Voxel> SunlightData::load(const AABB &region)
{
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
