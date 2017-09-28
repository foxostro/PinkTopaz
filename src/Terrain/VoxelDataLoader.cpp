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
#include "Profiler.hpp"

VoxelDataLoader::VoxelDataLoader()
: VOXEL_MAGIC('lxov'), VOXEL_VERSION(0)
{}

void VoxelDataLoader::retrieveDimensions(const std::vector<uint8_t> &bytes, AABB &box, glm::ivec3 &res)
{
    const Header &header = *((Header *)bytes.data());
    
    glm::vec3 half(header.w / 2.f, header.h / 2.f, header.d / 2.f);
    box = {
        half,
        half
    };
    res = glm::ivec3(header.w, header.h, header.d);
}

void VoxelDataLoader::load(const std::vector<uint8_t> &bytes, Array3D<Voxel> &output)
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
        half
    };
    
    size_t i = 0;
    
    output.mutableForEachCell(box, [&](const AABB &cell, Morton3 index, Voxel &voxel){
        assert(i < (header.w * header.h * header.d));
        const FileVoxel &src = header.voxels[i++];
        const float value = (src.type == 0) ? 0.0f : 1.0f;
        voxel = Voxel(value);
    });
}

Array3D<Voxel> VoxelDataLoader::createArray(const std::vector<uint8_t> &bytes, int border)
{
    AABB box;
    glm::ivec3 res;
    retrieveDimensions(bytes, box, res);
    
    const AABB boxWithBorder = box.inset(-glm::vec3(border, border, border));
    const glm::ivec3 resWithBorder = res + glm::ivec3(border, border, border)*2;
    
    Array3D<Voxel> voxels(boxWithBorder, resWithBorder);
    
    voxels.mutableForEachCell(boxWithBorder, [&](const AABB &cell, Morton3 index, Voxel &value){
        value = Voxel();
    });
    
    load(bytes, voxels);
    
    return voxels;
}
