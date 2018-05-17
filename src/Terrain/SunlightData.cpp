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
                           std::unique_ptr<MapRegionStore> &&mapRegionStore,
                           const std::shared_ptr<TaskDispatcher> &dispatcher)
: VoxelData(log,
            std::move(source),
            chunkSize,
            std::move(mapRegionStore),
            dispatcher),
  _noiseSource(std::make_unique<SimplexNoise>(0))
{}

SunlightData::ChunkPtr SunlightData::createNewChunk(const AABB &cell, Morton3 index)
{
    ChunkPtr chunk;
    _source->readerTransaction(cell, [&](const Array3D<Voxel> &voxels) {
        chunk = std::make_shared<Chunk>(voxels);
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

void SunlightData::writerTransaction(const std::shared_ptr<TerrainOperation> &operation)
{
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
    
    {
        auto mutex = _lockArbitrator.writerMutex(sunlightRegion);
        std::lock_guard<decltype(mutex)> lock(mutex);
        
        for (const auto chunkCellCoords : slice(_chunks, sunlightRegion)) {
            const glm::vec3 chunkCenter = _chunks.cellCenterAtCellCoords(chunkCellCoords);
            const Morton3 chunkIndex = _chunks.indexAtCellCoords(chunkCellCoords);
            _chunks.invalidate(chunkIndex);
            _mapRegionStore->invalidate(chunkCenter, chunkIndex);
        }
        
        _source->writerTransaction(operation);
    }
    
    onWriterTransaction(sunlightRegion);
}
