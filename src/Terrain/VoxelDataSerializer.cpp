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
#include <mutex>

static uint32_t computeChecksum(const std::vector<uint8_t> &input)
{
    // crc_32_type is not thread-safe. It has a static table which is
    // initialized on first access and then used on subsequent accesses.
    static std::mutex mutex;
    std::scoped_lock lock(mutex);
    
    boost::crc_32_type result;
    result.process_bytes(input.data(), input.size());
    return result.checksum();
}

static std::vector<uint8_t> compress(const std::vector<uint8_t> &input)
{
    // Guess how many compressed bytes. We'll guess the output has the same
    // number of bytes as the input, but it's probably smaller. That is, after
    // all, the whole point.
    const size_t guess = std::max(input.size(), (size_t)1);
    
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
                throw VoxelDataException("Z_MEM_ERROR");
                
            case Z_STREAM_ERROR:
                throw VoxelDataException("Z_STREAM_ERROR");
                
            default:
                assert(!"not reachable");
                throw VoxelDataException("Unknown result from zlib compress: {}", r);
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
                throw VoxelDataException("Z_MEM_ERROR");
                
            case Z_DATA_ERROR:
                throw VoxelDataException("Z_DATA_ERROR");
                
            default:
                assert(!"not reachable");
                throw VoxelDataException("Unknown result from zlib uncompress: {}", r);
        };
    }
    
    return output;
}

VoxelDataSerializer::VoxelDataSerializer()
 : VOXEL_MAGIC('lxov'), VOXEL_VERSION(2)
{}

VoxelDataChunk VoxelDataSerializer::load(const AABB &boundingBox,
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
    
    if (header.chunkType == CHUNK_TYPE_ARRAY &&
        (header.w == 0 || header.h == 0 || header.d == 0 ||
         header.w >= (1<<22) || header.h >= (1<<21) || header.d >= (1<<21))) {
        throw VoxelDataInvalidSizeException(header.w, header.h, header.d);
    }
    
    const glm::ivec3 gridResolution(header.w, header.h, header.d);
    
    bool complete = (header.complete != 0);
    
    switch (header.chunkType) {
        case CHUNK_TYPE_ARRAY:
        {
            const std::vector<uint8_t> compressedBytes(header.compressedBytes, header.compressedBytes + header.len);
            
            uint32_t s = computeChecksum(compressedBytes);
            if (header.checksum != s) {
                throw VoxelDataChecksumException(header.checksum, s);
            }
            
            const std::vector<uint8_t> decompressedBytes = decompress(compressedBytes);
            Array3D<Voxel> voxels(boundingBox, gridResolution);
            memcpy((void *)voxels.data(), (const void *)decompressedBytes.data(), decompressedBytes.size());
            
            auto chunk = VoxelDataChunk::createArrayChunk(std::move(voxels));
            chunk.complete = complete;
            return chunk;
        }
            
        case CHUNK_TYPE_SKY:
        {
            auto chunk = VoxelDataChunk::createSkyChunk(boundingBox, gridResolution);
            chunk.complete = complete;
            return chunk;
        }
            
        case CHUNK_TYPE_GROUND:
        {
            auto chunk = VoxelDataChunk::createGroundChunk(boundingBox, gridResolution);;
            chunk.complete = complete;
            return chunk;
        }
            
        default:
            throw VoxelDataException("Unknown voxel chunk type {}", header.chunkType);
    }
}

std::vector<uint8_t> VoxelDataSerializer::store(const VoxelDataChunk &chunk)
{
    const glm::ivec3 res = chunk.gridResolution();
    
    // Get the uncompressed voxel bytes from the chunk.
    std::vector<uint8_t> uncompressedBytes = chunk.getUncompressedBytes();
    
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
    
    switch (chunk.getType()) {
        case VoxelDataChunk::Array:
            header.chunkType = CHUNK_TYPE_ARRAY;
            break;
            
        case VoxelDataChunk::Sky:
            header.chunkType = CHUNK_TYPE_SKY;
            break;
            
        case VoxelDataChunk::Ground:
            header.chunkType = CHUNK_TYPE_GROUND;
            break;
            
        default:
            assert(!"unreachable");
    }
    
    header.complete = chunk.complete ? 1 : 0;
    
    memcpy((void *)header.compressedBytes,
           (const void *)compressedBytes.data(),
           compressedBytes.size());

    return serializedData;
}
