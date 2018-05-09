//
//  TerrainOperation.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#ifndef TerrainOperation_hpp
#define TerrainOperation_hpp

#include "VoxelData.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

// Base class for operations which modify voxel data.
class TerrainOperation
{
public:
    // Default destructor.
    virtual ~TerrainOperation() = default;
    
    // Gets the bounding box of the region affected by this operation.
    const AABB& getAffectedRegion() const
    {
        return _affectedRegion;
    }
    
    // Performs the operation.
    virtual void perform(Array3D<Voxel> &voxelData) = 0;
    
    // Serialize the operation.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(_affectedRegion);
    }
    
protected:
    // Sets the bounding box of the region affected by this operation.
    // Subclasses must ensure that the affected region is set prior to the
    // perform() method being called. This is used to determine the region of
    // the voxel data that must be locked for writing.
    void setAffectedRegion(AABB region)
    {
        _affectedRegion = region;
    }
    
private:
    AABB _affectedRegion;
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
