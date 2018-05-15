//
//  VoxelDataSource.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/14/18.
//
//

#ifndef VoxelDataSource_hpp
#define VoxelDataSource_hpp

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Voxel.hpp"

// A source for voxel data.
class VoxelDataSource : public GridIndexer
{
public:
    // Default Destructor.
    virtual ~VoxelDataSource() = default;
    
    // No default constructor.
    VoxelDataSource() = delete;
    
    // Constructor.
    // boundingBox -- The region for which voxels are defined.
    // gridResolution -- The resolution of the voxel grid.
    VoxelDataSource(const AABB &boundingBox,
                    const glm::ivec3 &gridResolution)
     : GridIndexer(boundingBox, gridResolution)
    {}
    
    // Loads a copy of the contents of the specified sub-region of the grid to
    // an Array3D and returns that.
    virtual Array3D<Voxel> load(const AABB &region) = 0;
};

#endif /* VoxelDataSource_hpp */
