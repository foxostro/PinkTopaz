//
//  VoxelDataLoader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/21/17.
//
//

#ifndef VoxelDataLoader_hpp
#define VoxelDataLoader_hpp

#include "Terrain/VoxelData.hpp"

namespace Terrain {
    
    class VoxelDataLoader
    {
    public:
        struct FileVoxel
        {
            // Cache the results of the calculation of whether this vertex is
            // outside or inside.
            uint8_t outside:1;
            
            // Indicates a torch is placed on this block. It is a light source.
            uint8_t torch:1;
            
            // Indicates the voxel transmits light as if it were air.
            uint8_t opaque:1;
            
            // The voxel type affects the mesh which is used when drawing it.
            uint8_t type:3;
            
            // This is the texture used on the top of the voxel.
            //This is used as an index into the terrain texture array.
            uint8_t texTop:3;
            
            // This is the texture used on the side of the voxel.
            // This is used as an index into the terrain texture array.
            uint8_t texSide:3;
        };
        
        struct Header
        {
            uint32_t magic;
            uint32_t version;
            uint32_t w, h, d;
            uint64_t len;
            FileVoxel voxels[0];
        };
        
        VoxelDataLoader();
        ~VoxelDataLoader() = default;
        
        VoxelData load(const char *filePath);
        
    private:
        const uint32_t VOXEL_MAGIC, VOXEL_VERSION;
    };
    
} // namespace Terrain

#endif /* StaticMesh_hpp */
