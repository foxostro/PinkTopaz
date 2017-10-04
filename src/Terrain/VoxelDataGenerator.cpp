//
//  VoxelDataGenerator.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/TerrainConfig.hpp"
#include "Noise/SimplexNoise.hpp"

using namespace glm;

static constexpr int size = 1024;
static constexpr int border = TERRAIN_CHUNK_SIZE;
static constexpr int extent = size + border;
static constexpr int res = (size + border) * 2;

// Return a value between -1 and +1 so that a line through the y-axis maps to a
// smooth gradient of values from -1 to +1.
static inline float groundGradient(float terrainHeight, const vec3 &p)
{
    if(p.y < 0.0f) {
        return -1.f;
    } else if(p.y > terrainHeight) {
        return +1.f;
    } else {
        return 2.0f * (p.y / terrainHeight) - 1.0f;
    }
}

// Generates a voxel for the specified point and returns it in `outVoxel'.
static void generateTerrainVoxel(const Noise &noiseSource0,
                                 const Noise &noiseSource1,
                                 float terrainHeight,
                                 const vec3 &p,
                                 Voxel &outVoxel)
{
    bool groundLayer = false;
    bool floatingMountain = false;
    
    // Normal rolling hills
    {
        const float freqScale = 0.025f;
        float n = noiseSource0.noiseAtPointWithFourOctaves(p * freqScale);
        float turbScaleX = 2.0f;
        float turbScaleY = terrainHeight / 2.0f;
        float yFreq = turbScaleX * ((n+1) / 2.0f);
        float t = turbScaleY * noiseSource1.noiseAtPoint(vec3(p.x*freqScale, p.y*yFreq*freqScale, p.z*freqScale));
        groundLayer = groundGradient(terrainHeight, vec3(p.x, p.y + t, p.z)) <= 0;
    }
    
    // Giant floating mountain
    {
        // The floating mountain is generated by starting with a sphere and
        // applying turbulence to the surface. The upper hemisphere is also
        // squashed to make the top flatter.
        
        vec3 mountainCenter(50.f, 50.f, 80.f);
        vec3 toMountainCenter = mountainCenter - p;
        float distance = length(toMountainCenter);
        float radius = 30.0f;
        
        // Apply turbulence to the surface of the mountain.
        float freqScale = 0.70f;
        float turbScale = 15.0f;
        
        // Avoid generating noise when too far away from the center to matter.
        if(distance > 2.0f * radius) {
            floatingMountain = false;
        } else {
            // Convert the point into spherical coordinates relative to the center of the mountain.
            float azimuthalAngle = acosf(toMountainCenter.z / distance);
            float polarAngle = atan2f(toMountainCenter.y, toMountainCenter.x);
            
            float t = turbScale * noiseSource0.noiseAtPointWithFourOctaves(vec3(azimuthalAngle * freqScale,
                                                                                polarAngle * freqScale,
                                                                                0.0f));
            
            // Flatten the top.
            if(p.y > mountainCenter.y) {
                radius -= (p.y - mountainCenter.y) * 3;
            }
            
            floatingMountain = (distance+t) < radius;
        }
    }
    
    outVoxel.value = (groundLayer || floatingMountain) ? 1.0f : 0.0f;
}

VoxelDataGenerator::VoxelDataGenerator(unsigned seed)
 : GridIndexer(AABB{glm::vec3(0.f, 0.f, 0.f), glm::vec3((float)extent, (float)extent, (float)extent)},
               glm::ivec3(res, res, res)),
               _noiseSource0(std::make_unique<SimplexNoise>(seed)),
               _noiseSource1(std::make_unique<SimplexNoise>(seed + 1))
{}

Array3D<Voxel> VoxelDataGenerator::copy(const AABB &region) const
{
    static constexpr float terrainHeight = 20.f;
    
    const AABB adjusted = snapRegionToCellBoundaries(region);
    const auto res = countCellsInRegion(adjusted);
    Array3D<Voxel> dst(adjusted, res);
    
    for (const auto &cellCoords : dst.slice(adjusted)) {
        const auto cellCenter = dst.cellCenterAtCellCoords(cellCoords);
        Voxel &value = dst.mutableReference(cellCoords);
        generateTerrainVoxel(*_noiseSource0,
                             *_noiseSource1,
                             terrainHeight,
                             cellCenter,
                             value);
    }
    
    return dst;
}
