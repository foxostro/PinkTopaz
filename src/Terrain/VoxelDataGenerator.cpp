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

VoxelDataGenerator::VoxelDataGenerator()
{
    // Create a voxel data store. We want to fill this with voxel values we read
    // from file. Before we can do that, we need to initialize the data store to
    // the dimensions of the voxel field found in the file.
    const std::vector<uint8_t> bytes = binaryFileContents("0_0_0.voxels.dat");
    AABB box;
    glm::ivec3 res;
    VoxelDataLoader voxelDataLoader;
    voxelDataLoader.retrieveDimensions(bytes, box, res);
    
    const int MESH_CHUNK_SIZE = 16;
    const glm::vec3 chunkSize(MESH_CHUNK_SIZE, MESH_CHUNK_SIZE, MESH_CHUNK_SIZE);
    const AABB boxWithBorder = box.inset(-chunkSize);
    const glm::ivec3 resWithBorder = res + glm::ivec3(MESH_CHUNK_SIZE, MESH_CHUNK_SIZE, MESH_CHUNK_SIZE)*2;
    
    _voxels = std::make_shared<Array3D<Voxel>>(boxWithBorder, resWithBorder);
    
    assert(_voxels->cellDimensions() == glm::vec3(1.0, 1.0, 1.0));
        
    _voxels->mutableForEachCell(boxWithBorder, [&](const AABB &cell){
        return Voxel();
    });
    
    voxelDataLoader.load(bytes, *_voxels);
}

const Voxel& VoxelDataGenerator::get(const glm::vec3 &p) const
{
    return _voxels->get(p);
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