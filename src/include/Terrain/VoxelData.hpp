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
class VoxelData : public GridAddressable<Voxel>
{
public:
    typedef Array3D<Voxel> Chunk;
    
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
    
    // Each point in space corresponds to exactly one cell. Get the object.
    Voxel get(const glm::vec3 &p) const override;
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    Voxel& getm(const glm::vec3 &p) override;
    
    // Each point in space corresponds to exactly one cell. Get the object.
    // If the point is not in bounds then return the specified default value.
    Voxel get(const glm::vec3 &p, const Voxel &defaultValue) const override;
    
    // Each point in space corresponds to exactly one cell. Set the object.
    void set(const glm::vec3 &p, const Voxel &object) override;
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    glm::vec3 getCellDimensions() const override;
    
    // Gets the region for which the grid is defined.
    AABB getBoundingBox() const override;
    
    // Gets the number of cells along each axis within the valid region.
    glm::ivec3 getResolution() const override;
    
private:
    Chunk _chunk;
};

#endif /* VoxelData_hpp */
