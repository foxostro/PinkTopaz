//
//  VoxelDataGenerator.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#include "Terrain/VoxelDataGenerator.hpp"
#include "Terrain/VoxelDataLoader.hpp"
#include "FileUtilities.hpp"

VoxelDataGenerator::VoxelDataGenerator(unsigned chunkSize)
{
    // Create a voxel data store. We want to fill this with voxel values we read
    // from file. Before we can do that, we need to initialize the data store to
    // the dimensions of the voxel field found in the file.
    const std::vector<uint8_t> bytes = binaryFileContents("0_0_0.voxels.dat");
    AABB box;
    glm::ivec3 res;
    VoxelDataLoader voxelDataLoader;
    voxelDataLoader.retrieveDimensions(bytes, box, res);
    
    const glm::vec3 chunkDim(chunkSize, chunkSize, chunkSize);
    const AABB boxWithBorder = box.inset(-chunkDim);
    const glm::ivec3 resWithBorder = res + glm::ivec3(chunkSize, chunkSize, chunkSize)*2;
    
    _voxels = std::make_shared<Array3D<Voxel>>(boxWithBorder, resWithBorder);
    
    assert(_voxels->cellDimensions() == glm::vec3(1.0, 1.0, 1.0));
        
    _voxels->mutableForEachCell(boxWithBorder, [&](const AABB &cell, Morton3 index, Voxel &value){
        value = Voxel();
    });
    
    voxelDataLoader.load(bytes, *_voxels);
}

const Voxel& VoxelDataGenerator::get(const glm::vec3 &p) const
{
    return _voxels->get(p);
}

const Voxel& VoxelDataGenerator::get(const glm::ivec3 &cellCoords) const
{
    return _voxels->get(cellCoords);
}

const Voxel& VoxelDataGenerator::get(Morton3 index) const
{
    return _voxels->get(index);
}

Array3D<Voxel> VoxelDataGenerator::copy(const AABB &region) const
{
    const AABB adjustedRegion = snapRegionToCellBoundaries(region);
    
    // Count the number of voxels in the adjusted region. This will be the grid
    // resolution of the destination array.
    const glm::ivec3 res = countCellsInRegion(adjustedRegion);
    
    // Construct the destination array.
    Array3D<Voxel> dst(adjustedRegion, res);
    assert(dst.inbounds(region));
    
    dst.mutableForEachCell(adjustedRegion, [&](const AABB &cell,
                                               Morton3 index,
                                               Voxel &value){
        // We need to use get(vec3) because the index is only valid within
        // this one chunk.
        // AFOX_TODO: Do something clever here with indices so reduce calls to indexAtPoint().
        value = get(cell.center);
    });
    
    return dst;
}

glm::vec3 VoxelDataGenerator::cellDimensions() const
{
    glm::vec3 dim = _voxels->cellDimensions();
    return dim;
}

AABB VoxelDataGenerator::boundingBox() const
{
    AABB box = _voxels->boundingBox();
    return box;
}

glm::ivec3 VoxelDataGenerator::gridResolution() const
{
    glm::ivec3 res = _voxels->gridResolution();
    return res;
}
