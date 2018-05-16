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

SunlightData::SunlightData(std::unique_ptr<VoxelDataSource> &&source,
                           unsigned chunkSize,
                           std::unique_ptr<MapRegionStore> &&mapRegionStore,
                           const std::shared_ptr<TaskDispatcher> &dispatcher)
: VoxelData(std::move(source),
            chunkSize,
            std::move(mapRegionStore),
            dispatcher),
  _noiseSource(std::make_unique<SimplexNoise>(0))
{}

SunlightData::ChunkPtr SunlightData::createNewChunk(const AABB &cell, Morton3 index)
{
    ChunkPtr chunk;
    getSource().readerTransaction(cell, [&](const Array3D<Voxel> &voxels) {
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

AABB SunlightData::getAffectedRegionForOperation(const std::shared_ptr<TerrainOperation> &operation)
{
    // TODO: Eventually, this should return a larger region where sunlight propagation may have occurred.
    return operation->getAffectedRegion();
}
