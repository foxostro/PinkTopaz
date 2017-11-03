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

#include "zlib.h"
#include <boost/crc.hpp>
#include <cstring>

static uint32_t computeChecksum(const std::vector<uint8_t> &input)
{
    boost::crc_32_type result;
    result.process_bytes(input.data(), input.size());
    return result.checksum();
}

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
 : VOXEL_MAGIC('lxov'), VOXEL_VERSION(2)
{}

Array3D<Voxel> VoxelDataSerializer::load(const AABB &boundingBox,
                                         const std::vector<uint8_t> &bytes)
{
    assert(bytes.size() > sizeof(Header));
    
    const Header &header = *((Header *)bytes.data());
    
    if (header.magic != VOXEL_MAGIC) {
        throw VoxelDataMagicNumberException(header.magic, VOXEL_MAGIC);
    }
    
    if (header.version != VOXEL_VERSION) {
        throw VoxelDataIncompatibleVersionException(header.version, VOXEL_VERSION);
    }
    
    if (header.w == 0 || header.h == 0 || header.d == 0 ||
        header.w > 255 || header.h > 255 || header.d > 255) {
        throw VoxelDataInvalidSizeException(header.w, header.h, header.d);
    }
    
    const std::vector<uint8_t> compressedBytes(header.compressedBytes, header.compressedBytes + header.len);
    
    uint32_t s = computeChecksum(compressedBytes);
    if (header.checksum != s) {
        throw VoxelDataChecksumException(header.checksum, s);
    }
    
    const std::vector<uint8_t> decompressedBytes = decompress(compressedBytes);
    const glm::ivec3 gridResolution(header.w, header.h, header.d);
    Array3D<Voxel> voxels(boundingBox, gridResolution);
    memcpy((void *)voxels.data(), (const void *)decompressedBytes.data(), decompressedBytes.size());
    
    return voxels;
}

std::vector<uint8_t> VoxelDataSerializer::store(const Array3D<Voxel> &voxels)
{
    const glm::ivec3 res = voxels.gridResolution();
    
    // Get the uncompressed voxel bytes from the chunk.
    const size_t numberOfVoxelBytes = res.x * res.y * res.z * sizeof(Voxel);
    std::vector<uint8_t> uncompressedBytes;
    uncompressedBytes.resize(numberOfVoxelBytes);
    memcpy((void *)uncompressedBytes.data(),
           (const void *)voxels.data(),
           numberOfVoxelBytes);
    
    // Compress the voxel data.
    const std::vector<uint8_t> compressedBytes = compress(uncompressedBytes);
    
    // Build the serialized voxel data structure and the header.
    std::vector<uint8_t> serializedData(compressedBytes.size() + sizeof(Header));
    
    Header &header = *((Header *)serializedData.data());
    header.magic = VOXEL_MAGIC;
    header.version = VOXEL_VERSION;
    header.checksum = computeChecksum(compressedBytes);
    header.w = res.x;
    header.h = res.y;
    header.d = res.z;
    header.len = (uint32_t)compressedBytes.size();
    
    memcpy((void *)header.compressedBytes,
           (const void *)compressedBytes.data(),
           compressedBytes.size());

    return serializedData;
}
