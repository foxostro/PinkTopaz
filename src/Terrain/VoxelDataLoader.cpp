//
//  VoxelDataLoader.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/21/17.
//
//

#include "SDL.h"
#include "FileUtilities.hpp"
#include "Exception.hpp"
#include "Terrain/VoxelDataLoader.hpp"

VoxelDataLoader::VoxelDataLoader()
: VOXEL_MAGIC('lxov'), VOXEL_VERSION(0)
{}

void VoxelDataLoader::retrieveDimensions(const std::vector<uint8_t> &bytes, AABB &box, glm::ivec3 &res)
{
    const Header &header = *((Header *)bytes.data());
    
    glm::vec3 half(header.w / 2.f, header.h / 2.f, header.d / 2.f);
    box = {
        half,
        half,
    };
    res = glm::ivec3(header.w, header.h, header.d);
}

void VoxelDataLoader::load(const std::vector<uint8_t> &bytes, VoxelData &output)
{
    const Header &header = *((Header *)bytes.data());
    
    if (header.magic != VOXEL_MAGIC) {
        throw Exception("Unexpected magic number in voxel data file: found %d but expected %d", header.magic, VOXEL_MAGIC);
    }
    
    if (header.version != VOXEL_VERSION) {
        throw Exception("Unexpected version number in voxel data file: found %d but expected %d", header.version, VOXEL_VERSION);
    }
    
    size_t expectedSize = header.w*header.h*header.d*sizeof(FileVoxel);
    if (header.len != expectedSize) {
        throw Exception("Unexpected number of bytes used in voxel data file: found %d, but expected %d", header.len, expectedSize);
    }
    
    glm::vec3 half(header.w / 2.f, header.h / 2.f, header.d / 2.f);
    AABB box = {
        half,
        half,
    };
    glm::ivec3 resolution(header.w, header.h, header.d);
    
    if (box != output.getBoundingBox()) {
        throw Exception("Unexpected voxel data bounding box used in voxel data file.");
    }
    
    if (resolution != output.getResolution()) {
        throw Exception("Unexpected voxel data resolution used in voxel data file.");
    }
    
    for (size_t i = 0, n = header.w*header.h*header.d; i < n; ++i)
    {
        const FileVoxel &src = header.voxels[i];
        float value = (src.type == 0) ? 0.0f : 1.0f;
        output.set(i, Voxel(value));
    }
}
