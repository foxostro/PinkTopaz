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
#include "Terrain/InitialSunlightPropagationOperation.hpp"

using namespace glm;

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

Array3D<Voxel> VoxelData::load(const AABB &region)
{
    {
        InitialSunlightPropagationOperation operation(_log, _chunks, _dispatcher);
        operation.performInitialSunlightPropagationIfNecessary(region);
    }
    
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
    _chunks.store(chunkIndex, _dispatcher);
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
