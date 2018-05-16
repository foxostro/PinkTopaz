//
//  TerrainOperation.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#ifndef TerrainOperation_hpp
#define TerrainOperation_hpp

#include "Voxel.hpp"
#include "Grid/Array3D.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

// Base class for operations which modify voxel data.
class TerrainOperation
{
public:
    // Default destructor.
    virtual ~TerrainOperation() = default;
    
    // Gets the bounding box of the region to be locked for writing.
    const AABB& getVoxelWriteRegion() const
    {
        return _voxelWriteRegion;
    }
    
    // Gets the region where meshes are affected by this operation.
    const AABB& getMeshEffectRegion() const
    {
        return _meshEffectRegion;
    }
    
    // Performs the operation.
    virtual void perform(Array3D<Voxel> &voxelData) = 0;
    
    // Serialize the operation.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(_voxelWriteRegion, _meshEffectRegion);
    }
    
protected:
    // Sets the bounding box of the region in the voxel grid to be locked for
    // writing in perform(). Subclasses must ensure that this is set prior to
    // the perform() method being called.
    void setVoxelWriteRegion(AABB region)
    {
        _voxelWriteRegion = region;
    }
    
    // Sets the bounding box of the region where meshes are affected by this
    // change.
    // Unlike the voxel write region, this may be set at any point before the
    // operation is entered into the journal.
    void setMeshEffectRegion(AABB region)
    {
        _meshEffectRegion = region;
    }
    
private:
    AABB _voxelWriteRegion;
    AABB _meshEffectRegion;
};

// Subclasses should make sure to declare the following to ensure Cereal knows
// about them:
//
//     CEREAL_REGISTER_TYPE(DerivedTerrainOperation)
//
// Subclasses should also make sure to declare a serialize method like so, in
// order to ensure the base class is serialized properly.
//
//     template<typename Archive>
//     void serialize(Archive &archive)
//     {
//         archive(cereal::base_class<TerrainOperation>(this), _aMember);
//     }
//

#endif /* TerrainOperation_hpp */
