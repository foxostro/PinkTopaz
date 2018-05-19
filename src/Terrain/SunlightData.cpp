//
//  SunlightData.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/15/18.
//
//

#include "Terrain/SunlightData.hpp"
#include "Grid/GridIndexerRange.hpp"
#include "Noise/SimplexNoise.hpp"
#include "math.hpp" // for clamp

SunlightData::SunlightData(std::shared_ptr<spdlog::logger> log,
                           std::unique_ptr<VoxelDataSource> &&source,
                           unsigned chunkSize,
                           std::unique_ptr<MapRegionStore> &&mapRegionStore)
: VoxelDataSource(source->boundingBox(), source->gridResolution()),
  _log(log),
  _source(std::move(source)),
  _noiseSource(std::make_unique<SimplexNoise>(0)),
  _chunks(log,
          _source->boundingBox(),
          _source->gridResolution(),
          chunkSize,
          std::move(mapRegionStore),
          [=](const AABB &cell){
              return createNewChunk(cell);
          })
{}

void SunlightData::readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn)
{
    fn(_chunks.load(region));
}

void SunlightData::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
    const AABB sunlightRegion = getAccessRegionForOperation(operation);
    _chunks.invalidate(sunlightRegion);
    _source->writerTransaction(operation);
}

void SunlightData::setWorkingSet(const AABB &workingSet)
{
    _source->setWorkingSet(workingSet);
    _chunks.setWorkingSet(workingSet);
}

std::unique_ptr<PersistentVoxelChunks::Chunk> SunlightData::createNewChunk(const AABB &cell)
{
    std::unique_ptr<PersistentVoxelChunks::Chunk> chunk;
    
    _source->readerTransaction(cell, [&](const Array3D<Voxel> &voxels) {
        chunk = std::make_unique<PersistentVoxelChunks::Chunk>(voxels);
    });
    
    Array3D<Voxel> &voxels = *chunk;
    for (const auto cellCoords : slice(voxels, voxels.boundingBox())) {
        const glm::vec3 point = voxels.cellCenterAtCellCoords(cellCoords);
        Voxel &voxel = voxels.mutableReference(cellCoords);
        
        // Generate some smooth noise and stick it in the lighting channel.
        // This is for testing and development and will be removed later.
        int sunLight = (int)clamp(_noiseSource->noiseAtPoint(point * 0.1f) * 8.f + 4.f, 0.f, 15.f);
        voxel.sunLight = sunLight;
    }
    
    return chunk;
}

AABB SunlightData::getAccessRegionForOperation(const std::shared_ptr<TerrainOperation> &operation)
{
    AABB sourceAccessRegion = _source->getAccessRegionForOperation(operation);
    
#if 0
    AABB sunlightRegion;
    {
        sunlightRegion = operation->getAffectedRegion();
        
        // First, account for horizontal propagation.
        sunlightRegion = sunlightRegion.inset(-glm::vec3(MAX_LIGHT));
        
        // Second, account for downward, vertical propagation.
        glm::vec3 mins = sunlightRegion.mins();
        glm::vec3 maxs = sunlightRegion.maxs();
        mins.y = boundingBox().mins().y;
        glm::vec3 center = (maxs + mins) * 0.5f;
        glm::vec3 extent = (maxs - mins) * 0.5f;
        sunlightRegion = {center, extent};
        
        sunlightRegion = boundingBox().intersect(sunlightRegion);
    }
#else
    AABB sunlightRegion = operation->getAffectedRegion();
#endif
    
    AABB combinedAccessRegion = boundingBox().intersect(sunlightRegion.unionBox(sourceAccessRegion));
    return combinedAccessRegion;
}
