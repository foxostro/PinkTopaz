//
//  VoxelDataSerializer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/3/17.
//
//

#ifndef VoxelDataSerializer_hpp
#define VoxelDataSerializer_hpp

#include "Terrain/Voxel.hpp"
#include "Grid/Array3D.hpp"
#include "Exception.hpp"

class VoxelDataException : public Exception
{
public:
    template<typename... Args>
    VoxelDataException(Args&&... args)
     : Exception(std::forward<Args>(args)...)
    {}
};

class VoxelDataMagicNumberException : public VoxelDataException
{
public:
    VoxelDataMagicNumberException(unsigned expected, unsigned actual)
     : VoxelDataException("Voxel Data magic number is incorrect. Expected %u, Got %u", expected, actual)
    {}
};

class VoxelDataChecksumException : public VoxelDataException
{
public:
    VoxelDataChecksumException(unsigned expected, unsigned actual)
     : VoxelDataException("Voxel Data checksums do not match. Expected %u, Got %u", expected, actual)
    {}
};

class VoxelDataIncompatibleVersionException : public Exception
{
public:
    VoxelDataIncompatibleVersionException(unsigned expected, unsigned actual)
     : Exception("Voxel Data versions do not match. Expected %u, Got %u",
                 expected, actual)
    {}
};

class VoxelDataInvalidSizeException : public Exception
{
public:
    VoxelDataInvalidSizeException(unsigned w, unsigned h, unsigned d)
     : Exception("Voxel Data dimensions are invalid: %u x %u x %u", w, h, d)
    {}
};

// Serializes a chunk of voxels to a sequence of bytes, and back.
class VoxelDataSerializer
{
public:
    // This header is placed at the beginning of the serialized voxel data.
    struct Header {
        // Magic number to identify the voxels. This is a very simple way to
        // check data integrity and makes it easy to find the header in a hex
        // editor.
        uint32_t magic;
        
        // Version number for the serialized voxel data.
        uint32_t version;
        
        // CRC32 checksum of the compressed voxel data.
        uint32_t checksum;
        
        // The dimensions of the voxel grid.
        uint32_t w, h, d;
        
        // The number of compressed bytes in `compressedBytes'.
        uint32_t len;
        
        // The compressed voxel bytes.
        uint8_t compressedBytes[0];
    };
    
    VoxelDataSerializer();
    ~VoxelDataSerializer() = default;
    
    // Create a chunk of voxels from the provided sequence of bytes.
    // The bounding box is specified here as that information is not stored in
    // the serialized representation of the chunk. (This lets serialized chunks
    // be moved in space.)
    Array3D<Voxel> load(const AABB &boundingBox,
                        const std::vector<uint8_t> &bytes);
    
    // Serialize the provided voxel chunk to a sequence of bytes.
    // The serialized representation is not guaranteed to be portable across
    // different systems. For example, we do nothing to address endianness or
    // differences in struct member alignment.
    std::vector<uint8_t> store(const Array3D<Voxel> &voxels);
    
private:
    const uint32_t VOXEL_MAGIC, VOXEL_VERSION;
};

#endif /* VoxelDataSerializer_hpp */
