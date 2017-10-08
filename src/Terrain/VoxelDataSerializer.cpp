//
//  VoxelDataSerializer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/3/17.
//
//

#include "Terrain/VoxelDataSerializer.hpp"
#include "Terrain/TerrainConfig.hpp"
#include "Terrain/Voxel.hpp"
#include "Exception.hpp"
#include "zlib.h"

static std::vector<uint8_t> compress(const std::vector<uint8_t> &input)
{
    // Guess how many compressed bytes. We'll guess the output has the same
    // number of bytes as the input, but it's probably smaller. That is, after
    // all, the whole point.
    const size_t guess = input.size();
    
    bool done = false;
    std::vector<uint8_t> output(guess);
    output.resize(guess);
    
    while (!done) {
        Bytef *dest = (Bytef *)&output[0];
        uLongf destLen = output.size();
        const Bytef *source = (const Bytef *)&input[0];
        uLong sourceLen = input.size();
        int r = compress(dest, &destLen, source, sourceLen);
        
        switch (r) {
            case Z_OK:
                done = true;
                output.resize(destLen);
                output.shrink_to_fit();
                break;
                
            case Z_BUF_ERROR:
                output.resize(output.size() * 2);
                break;
                
            case Z_MEM_ERROR:
                throw Exception("Z_MEM_ERROR");
                
            default:
                assert(!"not reachable");
                throw Exception("Unknown result from zlib compress: %d", r);
        };
    }
    
    return output;
}

static std::vector<uint8_t> decompress(const std::vector<uint8_t> &input)
{
    // Guess how many decompressed bytes. We're guessing the number of bytes
    // for a voxel chunk plus header.
    constexpr size_t guess = TERRAIN_CHUNK_SIZE*TERRAIN_CHUNK_SIZE*TERRAIN_CHUNK_SIZE*sizeof(Voxel) + sizeof(VoxelDataSerializer::Header);
    
    bool done = false;
    std::vector<uint8_t> output(guess);
    output.resize(guess);
    
    while (!done) {
        Bytef *dest = (Bytef *)&output[0];
        uLongf destLen = output.size();
        const Bytef *source = (const Bytef *)&input[0];
        uLong sourceLen = input.size();
        int r = uncompress(dest, &destLen, source, sourceLen);
        
        switch (r) {
            case Z_OK:
                done = true;
                output.resize(destLen);
                output.shrink_to_fit();
                break;
                
            case Z_BUF_ERROR:
                output.resize(output.size() * 2);
                break;
                
            case Z_MEM_ERROR:
                throw Exception("Z_MEM_ERROR");
                
            case Z_DATA_ERROR:
                throw Exception("Z_DATA_ERROR");
                
            default:
                assert(!"not reachable");
                throw Exception("Unknown result from zlib uncompress: %d", r);
        };
    }
    
    return output;
}

VoxelDataSerializer::VoxelDataSerializer()
 : VOXEL_MAGIC('lxov'), VOXEL_VERSION(1)
{}

Array3D<Voxel> VoxelDataSerializer::load(const AABB &boundingBox,
                                         const std::vector<uint8_t> &bytesPlusLength)
{
    assert(bytesPlusLength.size() > sizeof(uint32_t));
    const uint32_t size = *((uint32_t *)&bytesPlusLength[0]);
    assert(size < bytesPlusLength.size());
    std::vector<uint8_t> compressedBytes;
    compressedBytes.insert(compressedBytes.end(),
                           bytesPlusLength.begin() + sizeof(uint32_t),
                           bytesPlusLength.begin() + sizeof(uint32_t) + size);
    
    std::vector<uint8_t> decompressedBytes = decompress(compressedBytes);
    const Header &header = *((Header *)decompressedBytes.data());
    
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
    const glm::ivec3 res = voxels.gridResolution();
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
    
    auto compressedBytes = compress(bytes);
    
    // Add a four byte length to the beginning so we can get the uncompressed
    // bytes back later.
    const size_t compressedBytesSize = compressedBytes.size();
    assert(compressedBytesSize < UINT32_MAX);
    compressedBytes.insert(compressedBytes.begin(), 0);
    compressedBytes.insert(compressedBytes.begin(), 0);
    compressedBytes.insert(compressedBytes.begin(), 0);
    compressedBytes.insert(compressedBytes.begin(), 0);
    *((uint32_t *)&compressedBytes[0]) = (uint32_t)compressedBytesSize;
    
    return compressedBytes;
}
