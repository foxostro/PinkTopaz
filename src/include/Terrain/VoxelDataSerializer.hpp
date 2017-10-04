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

// Serializes a chunk of voxels to a sequence of bytes, and back.
class VoxelDataSerializer
{
public:
    struct Header {
        uint32_t magic;
        uint32_t version;
        uint32_t w, h, d;
        uint64_t len;
        Voxel voxels[0];
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
