//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "AABB.hpp"
#include "Voxel.hpp"

#include <vector>
#include <glm/vec3.hpp>

namespace Terrain {
    
    // A block of voxels in space.
    class VoxelData
    {
    public:
        // Constructor. Accepts a bounding box decribing the region of space
        // this block of voxels represents. The space is divided into voxel
        // cells at a resolution described by `resolution.' That is, there are
        // `resolution.x' voxel cells along the X-axis, `resolution.y' voxel
        // cells along the Y-axis, and `resolution.z' cells along the Z-axis.
        VoxelData(const AABB &box, const glm::ivec3 &resolution);
        
        // No default constructor.
        VoxelData() = delete;
        
        // Copy constructor is just the default.
        VoxelData(const VoxelData &voxels) = default;
        
        // Move constructor is just the default.
        VoxelData(VoxelData &&voxels) = default;
        
        // Destructor is just the default.
        ~VoxelData() = default;
        
        // Each point in space corresponds to exactly one voxel. Get that voxel.
        const Voxel& get(const glm::vec3 &p) const;
        
        // Each point in space corresponds to exactly one voxel. Set that voxel.
        void set(const glm::vec3 &p, const Voxel &voxel);
        
        // Gets the internal voxel index for the specified point in space.
        size_t indexAtPoint(const glm::vec3 &p) const;
        
        // Gets the voxel for the specified index, produced by `indexAtPoint'.
        const Voxel& get(size_t index) const;
        
        // Sets the voxel for the specified index, produced by `indexAtPoint'.
        void set(size_t index, const Voxel &voxel);

        // Gets the dimensions of a single voxel. (All are the same size.) 
        inline glm::vec3 getVoxelDimensions() const
        {
            return glm::vec3(_box.extent.x * 2.0f / _res.x,
                             _box.extent.y * 2.0f / _res.y,
                             _box.extent.z * 2.0f / _res.z);
        }
        
        inline const AABB& getBoundingBox() const { return _box; }
        
    private:
        std::vector<Voxel> _voxels;
        const AABB _box;
        const glm::ivec3 _res;
    };
    
} // namespace Terrain

#endif /* VoxelData_hpp */
