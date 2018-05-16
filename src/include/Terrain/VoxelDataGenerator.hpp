//
//  VoxelDataGenerator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#ifndef VoxelDataGenerator_hpp
#define VoxelDataGenerator_hpp

#include "Terrain/VoxelDataSource.hpp"
#include "Noise/Noise.hpp"

// Procedurally generates voxel data from pseudorandom noise.
class VoxelDataGenerator : public VoxelDataSource
{
public:
    VoxelDataGenerator(unsigned seed);
    VoxelDataGenerator() = delete;
    ~VoxelDataGenerator() = default;
    
    void readerTransaction(const AABB &region, std::function<void(const Array3D<Voxel> &data)> fn) override;
    
    // This operation is unavailable. Calling this will throw VoxelDataReadOnlyException.
    void writerTransaction(const std::shared_ptr<TerrainOperation> &operation) override
    {
        throw VoxelDataReadOnlyException();
    }
    
    void setWorkingSet(const AABB &workingSet) override {}
    
private:
    std::unique_ptr<Noise> _noiseSource0;
    std::unique_ptr<Noise> _noiseSource1;
    
    // Loads a copy of the contents of the specified sub-region of the grid to
    // an Array3D and returns that. May fault in missing voxels to satisfy the
    // request.
    Array3D<Voxel> load(const AABB &region);
};

#endif /* VoxelDataGenerator_hpp */
