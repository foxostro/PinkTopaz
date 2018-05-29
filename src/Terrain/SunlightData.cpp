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
    
    std::queue<glm::vec3> sunlightQueue;
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
    iterateColumns([&](const ivec3 &cellCoords){
        AABB chunkBoundingBox = chunkIndexer.cellAtCellCoords(cellCoords);
        Morton3 chunkIndex(cellCoords);
        auto chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex);
        assert(chunkPtr);
        seedSunlightInTopLayer(*chunkPtr, sunlightQueue);
        _log->info("SunlightData -- Seeded sunlight for chunk at {}", chunkBoundingBox);
    });
    
    _log->info("SunlightData -- Performing sunlight floodfill propagation.");
    _log->info("SunlightData -- sunlightQueue contains {} items.", sunlightQueue.size());
    while (!sunlightQueue.empty()) {
        const glm::vec3 voxelPos = sunlightQueue.front();
        sunlightQueue.pop();
        _log->info("SunlightData -- processing item: voxelPos={}", glm::to_string(voxelPos));
//        floodNeighbor(voxelPos, vec3(-1,  0,  0), sunlightQueue, false);
//        floodNeighbor(voxelPos, vec3(+1,  0,  0), sunlightQueue, false);
//        floodNeighbor(voxelPos, vec3( 0,  0, -1), sunlightQueue, false);
//        floodNeighbor(voxelPos, vec3( 0,  0, +1), sunlightQueue, false);
        floodNeighbor(voxelPos, vec3( 0, -1,  0), sunlightQueue, true);
    }
    
    _log->info("SunlightData -- Finished generation of sunlight data.");
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

void SunlightData::seedSunlightInTopLayer(VoxelDataChunk &chunk,
                                          std::queue<glm::vec3> &sunlightQueue)
{
    const ivec3 res = chunk.gridResolution();
    for (ivec3 cellCoords{0, res.y-1, 0}; cellCoords.x < res.x; ++cellCoords.x) {
        for (cellCoords.z = 0; cellCoords.z < res.z; ++cellCoords.z) {
            Voxel voxel = chunk.get(cellCoords);
            if (voxel.value == 0) {
                voxel.sunLight = MAX_LIGHT;
                chunk.set(cellCoords, voxel);
                sunlightQueue.emplace(chunk.cellCenterAtCellCoords(cellCoords));
            }
        }
    }
}

Voxel SunlightData::getVoxelAtPoint(const vec3 voxelPos)
{
    const GridIndexer &indexer = _chunks.getChunkIndexer();
    const ivec3 chunkIndex = indexer.cellCoordsAtPoint(voxelPos);
    const AABB chunkBoundingBox = indexer.cellAtCellCoords(chunkIndex);
    std::shared_ptr<VoxelDataChunk> chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex);
    assert(chunkPtr);
    const Voxel voxel = chunkPtr->get(voxelPos);
    return voxel;
}

void SunlightData::setVoxelAtPoint(const vec3 voxelPos, const Voxel &voxel)
{
    const GridIndexer &indexer = _chunks.getChunkIndexer();
    const ivec3 chunkIndex = indexer.cellCoordsAtPoint(voxelPos);
    const AABB chunkBoundingBox = indexer.cellAtCellCoords(chunkIndex);
    std::shared_ptr<VoxelDataChunk> chunkPtr = _chunks.get(chunkBoundingBox, chunkIndex);
    assert(chunkPtr);
    chunkPtr->set(voxelPos, voxel);
}

void SunlightData::floodNeighbor(const vec3 voxelPos,
                                 const vec3 delta,
                                 std::queue<glm::vec3> &sunlightQueue,
                                 bool losslessPropagationOfMaxLight)
{
    const Voxel nodeVoxel = getVoxelAtPoint(voxelPos);
    assert(nodeVoxel.value == 0);
    const vec3 neighborPos = voxelPos + delta;
    Voxel neighborVoxel = getVoxelAtPoint(neighborPos);
    
    if ((neighborVoxel.value == 0) && (nodeVoxel.value == 0)) {
        neighborVoxel.sunLight = MAX_LIGHT;
        setVoxelAtPoint(neighborPos, neighborVoxel);
        sunlightQueue.emplace(neighborPos);
    }
    
//    if ((neighborVoxel.value == 0) && ((neighborVoxel.sunLight + 2) <= nodeVoxel.sunLight)) {
//        if (losslessPropagationOfMaxLight && nodeVoxel.sunLight == MAX_LIGHT) {
//            neighborVoxel.sunLight = MAX_LIGHT;
//        } else {
//            neighborVoxel.sunLight = nodeVoxel.sunLight - 1;
//        }
//        setVoxelAtPoint(neighborPos, neighborVoxel);
//        sunlightQueue.emplace(neighborPos);
//    }
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
