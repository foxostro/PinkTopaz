//
//  VoxelDataGenerator.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/11/17.
//
//

#ifndef VoxelDataGenerator_hpp
#define VoxelDataGenerator_hpp

#include "Grid/GridIndexer.hpp"
#include "Grid/Array3D.hpp"
#include "Voxel.hpp"
#include "Noise/Noise.hpp"
#include <memory>

// Procedurally generates voxel data from pseudorandom noise.
class VoxelDataGenerator : public GridIndexer
{
public:
    VoxelDataGenerator(unsigned seed);
    VoxelDataGenerator() = delete;
    ~VoxelDataGenerator() = default;
    
    // Returns an array which holds a copy of the contents of the subregion.
    Array3D<Voxel> copy(const AABB &region) const;
    
private:
    std::unique_ptr<Noise> _noiseSource0;
    std::unique_ptr<Noise> _noiseSource1;
};

#endif /* VoxelDataGenerator_hpp */
