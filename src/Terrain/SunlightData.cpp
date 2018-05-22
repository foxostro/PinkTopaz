//
//  SunlightData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#include "Terrain/SunlightData.hpp"
#include "Grid/GridIndexerRange.hpp"

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
{}

Array3D<Voxel> SunlightData::load(const AABB &region)
{
    return _chunks.load(region);
}

void SunlightData::modify(TerrainOperation &operation)
{
    const AABB sunlightRegion = getAccessRegionForOperation(operation);
    _chunks.invalidate(sunlightRegion);
    _source->modify(operation);
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

std::unique_ptr<PersistentVoxelChunks::Chunk>
SunlightData::createNewChunk(const AABB &cell, Morton3 chunkIndex)
{
    Array3D<Voxel> voxels = _source->load(cell);
    
    const glm::ivec3 res = voxels.gridResolution();
    
    auto iterateHorizontalSlice = [&](std::function<void(glm::ivec3 cellCoords)> fn){
        for (glm::ivec3 cellCoords{0,0,0}; cellCoords.x < res.x; ++cellCoords.x) {
            for (cellCoords.z = 0; cellCoords.z < res.z; ++cellCoords.z) {
                fn(cellCoords);
            }
        }
    };
    
    // Get the top chunk. Then, copy the sunlight data at the bottom of the
    // top chunk to the top of this chunk.
    // If we're at the top of the world then there is no top chunk. In this case
    // we instead set the open voxels in the top slice to MAX_LIGHT.
    AABB topCell = cell;
    topCell.center.y += (2.f * cell.extent.y);
    if (inbounds(topCell)) {
        Morton3 topIndex = chunkIndex;
        topIndex.incY();
        auto topSunlightChunk = _chunks.get(topCell, topIndex);
        
        iterateHorizontalSlice([&](glm::ivec3 cellCoords){
            cellCoords.y = 0;
            const Voxel &topVoxel = topSunlightChunk->reference(cellCoords);
            cellCoords.y = voxels.gridResolution().y - 1;
            Voxel &voxel = voxels.mutableReference(cellCoords);
            if ((topVoxel.value == 0) && (voxel.value == 0)) {
                voxel.sunLight = topVoxel.sunLight;
            }
        });
    } else {
        iterateHorizontalSlice([&](glm::ivec3 cellCoords){
            cellCoords.y = voxels.gridResolution().y - 1;
            Voxel &voxel = voxels.mutableReference(cellCoords);
            if (voxel.value == 0) {
                voxel.sunLight = MAX_LIGHT;
            }
        });
    }
    
    iterateHorizontalSlice([&](glm::ivec3 cellCoords){
        for (cellCoords.y = res.y-2; cellCoords.y >= 0; --cellCoords.y) {
            const Voxel &topVoxel = voxels.reference(glm::ivec3(cellCoords.x, cellCoords.y + 1, cellCoords.z));
            Voxel &voxel = voxels.mutableReference(cellCoords);
            if ((topVoxel.value == 0) && (voxel.value == 0)) {
                voxel.sunLight = topVoxel.sunLight;
            }
        }
    });
    
    return std::make_unique<PersistentVoxelChunks::Chunk>(std::move(voxels));\
}

AABB SunlightData::getSunlightRegion(AABB sunlightRegion) const
{
    // First, account for horizontal propagation.
    sunlightRegion = sunlightRegion.inset(-glm::vec3(MAX_LIGHT));
    
    // Second, account for downward, vertical propagation.
    glm::vec3 mins = sunlightRegion.mins();
    glm::vec3 maxs = sunlightRegion.maxs();
    mins.y = boundingBox().mins().y;
    glm::vec3 center = (maxs + mins) * 0.5f;
    glm::vec3 extent = (maxs - mins) * 0.5f;
    sunlightRegion = {center, extent};
    
    // Clip against the bounding box of the sunlight grid.
    sunlightRegion = boundingBox().intersect(sunlightRegion);
    
    return sunlightRegion;
}
