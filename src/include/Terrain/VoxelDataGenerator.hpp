//
//  VoxelDataGenerator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#ifndef VoxelDataGenerator_hpp
#define VoxelDataGenerator_hpp

#include "GridAddressable.hpp"
#include "Array3D.hpp"
#include "Voxel.hpp"
#include "SimplexNoise.hpp"
#include <memory>

class VoxelDataGenerator : public GridIndexer
{
public:
    VoxelDataGenerator(unsigned seed);
    VoxelDataGenerator() = delete;
    ~VoxelDataGenerator() = default;
    
    // Returns an array which holds a copy of the contents of the subregion.
    Array3D<Voxel> copy(const AABB &region) const;
    
private:
    SimplexNoise _noiseSource0;
    SimplexNoise _noiseSource1;
};

#endif /* VoxelDataGenerator_hpp */
