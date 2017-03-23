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

namespace PinkTopaz::Terrain {
    
    VoxelDataLoader::VoxelDataLoader()
     : VOXEL_MAGIC('lxov'), VOXEL_VERSION(0)
    {}
    
    VoxelData VoxelDataLoader::load(const boost::filesystem::path &path)
    {
        std::vector<uint8_t> bytes = binaryFileContents(path);
        
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
            .center = half,
            .extent = half,
        };
        glm::ivec3 resolution(header.w, header.h, header.d);
        VoxelData voxels(box, resolution);
        
        for (size_t i = 0, n = header.w*header.h*header.d; i < n; ++i)
        {
            const FileVoxel &src = header.voxels[i];
            float value = (src.type == 0) ? 0.0f : 1.0f;
            voxels.set(i, Voxel(value));
        }
        
        return voxels;
    }

} // namespace PinkTopaz::Terrain
