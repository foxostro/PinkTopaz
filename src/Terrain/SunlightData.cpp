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
    return _chunks.loadSubRegion(region);
}

void SunlightData::editSingleVoxel(const glm::vec3 &point, const Voxel &value)
{
    const AABB sunlightRegion = getSunlightRegion({point, glm::vec3(0.1f)});
    _chunks.invalidate(sunlightRegion);
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
    VoxelDataChunk voxels = _source->load(cell);
    
    // If this chunk is Sky or Ground then it already has valid light data and
    // there's nothing to do.
    if (auto type = voxels.getType();
        type == VoxelDataChunk::Sky || type == VoxelDataChunk::Ground) {
        return std::make_unique<VoxelDataChunk>(std::move(voxels));
    }
    
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
    // If the top chunk is sky then we also know there is no terrain above us
    // and can set the open voxels in the top slice to MAX_LIGHT.
    bool shouldSeedInitialSunlightValues = true;
    AABB topCell = cell;
    topCell.center.y += (2.f * cell.extent.y);
    if (inbounds(topCell)) {
        Morton3 topIndex = chunkIndex;
        topIndex.incY();
        auto topSunlightChunk = _chunks.get(topCell, topIndex);
        
        if (topSunlightChunk->getType() != VoxelDataChunk::Sky) {
            shouldSeedInitialSunlightValues = false;
            iterateHorizontalSlice([&](glm::ivec3 cellCoords){
                cellCoords.y = 0;
                const Voxel topVoxel = topSunlightChunk->get(cellCoords);
                cellCoords.y = voxels.gridResolution().y - 1;
                Voxel voxel = voxels.get(cellCoords);
                if ((topVoxel.value == 0) && (voxel.value == 0)) {
                    voxel.sunLight = topVoxel.sunLight;
                    voxels.set(cellCoords, voxel);
                }
            });
        }
    }
    
    if (shouldSeedInitialSunlightValues) {
        iterateHorizontalSlice([&](glm::ivec3 cellCoords){
            cellCoords.y = voxels.gridResolution().y - 1;
            Voxel voxel = voxels.get(cellCoords);
            if (voxel.value == 0) {
                voxel.sunLight = MAX_LIGHT;
                voxels.set(cellCoords, voxel);
            }
        });
    }
    
    iterateHorizontalSlice([&](glm::ivec3 cellCoords){
        for (cellCoords.y = res.y-2; cellCoords.y >= 0; --cellCoords.y) {
            const Voxel topVoxel = voxels.get(glm::ivec3(cellCoords.x, cellCoords.y + 1, cellCoords.z));
            Voxel voxel = voxels.get(cellCoords);
            if ((topVoxel.value == 0) && (voxel.value == 0)) {
                voxel.sunLight = topVoxel.sunLight;
                voxels.set(cellCoords, voxel);
            }
        }
    });
    
    return std::make_unique<VoxelDataChunk>(std::move(voxels));
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
