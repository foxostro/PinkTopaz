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
    
    Array3D<Voxel> load(const AABB &boundingBox,
                        const std::vector<uint8_t> &bytes);
    
    std::vector<uint8_t> store(const Array3D<Voxel> &voxels);
    
private:
    const uint32_t VOXEL_MAGIC, VOXEL_VERSION;
};

#endif /* VoxelDataSerializer_hpp */
