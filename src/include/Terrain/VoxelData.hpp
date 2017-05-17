//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Array3D.hpp"
#include "Voxel.hpp"

// A block of voxels in space.
class VoxelData : public Array3D<Voxel>
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
};

#endif /* VoxelData_hpp */
