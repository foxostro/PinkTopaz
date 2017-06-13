//
//  VoxelDataGenerator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#ifndef VoxelDataGenerator_hpp
#define VoxelDataGenerator_hpp

#include "Array3D.hpp"
#include "Voxel.hpp"
#include <memory>

class VoxelDataGenerator : public GridAddressable<Voxel>
{
public:
    VoxelDataGenerator();
    ~VoxelDataGenerator() = default;
    
    // Get the object corresponding to the specified point in space.
    // Note that each point in space corresponds to exactly one cell.
    // Throws an exception if the point is not within this grid.
    const Voxel& get(const glm::vec3 &p) const override;
    
    // Gets the dimensions of a single cell in the grid.
    // Note that cells in the grid are always the same size.
    glm::vec3 cellDimensions() const override;
    
    // Gets the region for which the grid is defined.
    // Accesses to points outside this box is not permitted.
    AABB boundingBox() const override;
    
    // Gets the number of cells along each axis within the valid region.
    glm::ivec3 gridResolution() const override;
    
private:
    std::shared_ptr<Array3D<Voxel>> _voxels;
};

#endif /* VoxelDataGenerator_hpp */