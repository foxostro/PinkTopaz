//
//  VoxelDataGenerator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#ifndef VoxelDataGenerator_hpp
#define VoxelDataGenerator_hpp

#include "VoxelDataSource.hpp"
#include "Noise/Noise.hpp"

class VoxelDataGenerator : public VoxelDataSource
{
public:
    VoxelDataGenerator(unsigned seed);
    VoxelDataGenerator() = delete;
    ~VoxelDataGenerator() = default;
    
    // Returns an array which holds a copy of the contents of the subregion.
    Array3D<Voxel> load(const AABB &region) override;
    
private:
    std::unique_ptr<Noise> _noiseSource0;
    std::unique_ptr<Noise> _noiseSource1;
};

#endif /* VoxelDataGenerator_hpp */
