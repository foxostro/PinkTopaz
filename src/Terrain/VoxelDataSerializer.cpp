//
//  VoxelDataSerializer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/3/17.
//
//

#include "SDL.h"
#include "FileUtilities.hpp"
#include "Exception.hpp"
#include "Terrain/VoxelDataSerializer.hpp"

VoxelDataSerializer::VoxelDataSerializer()
 : VOXEL_MAGIC('lxov'), VOXEL_VERSION(1)
{}

Array3D<Voxel> VoxelDataSerializer::load(const AABB &boundingBox,
                                         const std::vector<uint8_t> &bytes)
{
    const Header &header = *((Header *)bytes.data());
    
    if (header.magic != VOXEL_MAGIC) {
        throw Exception("Unexpected magic number in voxel data file: found %d but expected %d", header.magic, VOXEL_MAGIC);
    }
    
    if (header.version != VOXEL_VERSION) {
        throw Exception("Unexpected version number in voxel data file: found %d but expected %d", header.version, VOXEL_VERSION);
    }
    
    if (header.len != (header.w * header.h * header.d * sizeof(Voxel))) {
        throw Exception("Unexpected number of bytes used in voxel data file.");
    }

    const glm::ivec3 gridResolution(header.w, header.h, header.d);
    Array3D<Voxel> voxels(boundingBox, gridResolution);
    memcpy((void *)voxels.data(), (const void *)header.voxels, header.len);
    return voxels;
}

std::vector<uint8_t> VoxelDataSerializer::store(const Array3D<Voxel> &voxels)
{
    glm::ivec3 res = voxels.gridResolution();
    
    const size_t numberOfVoxelBytes = res.x * res.y * res.z * sizeof(Voxel);
    std::vector<uint8_t> bytes(numberOfVoxelBytes + sizeof(Header));
    
    Header &header = *((Header *)bytes.data());
    header.magic = VOXEL_MAGIC;
    header.version = VOXEL_VERSION;
    header.w = res.x;
    header.h = res.y;
    header.d = res.z;
    header.len = numberOfVoxelBytes;
    
    memcpy((void *)header.voxels, (const void *)voxels.data(), header.len);
    
    return bytes;
}
