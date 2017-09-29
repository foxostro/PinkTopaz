//
//  VoxelData.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/17.
//
//

#ifndef VoxelData_hpp
#define VoxelData_hpp

#include "Grid/SparseGrid.hpp"
#include "Voxel.hpp"
#include "VoxelDataGenerator.hpp"
#include <memory>

// A block of voxels in space.
class VoxelData : public GridIndexer
{
public:
    using Chunk = Array3D<Voxel>;
    using MaybeChunk = boost::optional<Chunk>;
    using GeneratorPtr = std::shared_ptr<VoxelDataGenerator>;
    
    // Default Destructor.
    ~VoxelData() = default;
    
    // No default constructor.
    VoxelData() = delete;
    
    // Constructor.
    // generator -- The generator provides initial voxel data.
    // chunkSize -- The size of chunk VoxelData should use internally.
    VoxelData(const GeneratorPtr &generator, unsigned chunkSize);
    
    // Loads a copy of the contents of the specified sub-region of the grid to
    // an Array3D and returns that. May fault in missing voxels to satisfy the
    // request.
    Array3D<Voxel> load(const AABB &region);
    
    // Stores the contents of the specified array of voxels to the grid.
    void store(const Array3D<Voxel> &voxels);
    
private:
    const GeneratorPtr &_generator;
    Array3D<MaybeChunk> _chunks;
};

#endif /* VoxelData_hpp */
