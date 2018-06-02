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

class VoxelData;

// Base class for operations which modify voxel data.
class TerrainOperation
{
public:
    // Default destructor.
    virtual ~TerrainOperation() = default;
    
    // Default constructor.
    // You probably don't want this unless you're deserializing the operation
    // from file. There are two ways to set the affected region: 1) the ctor,
    // and 2) the serialize() method.
    TerrainOperation() = default;
    
    // Constructor.
    // affectedRegion -- All changes made by the operation are contained within
    //                   this bounding box.
    TerrainOperation(const AABB &affectedRegion)
    : _affectedRegion(affectedRegion)
    {}
    
    // Returns a bounding box which contains all changes made by by operation.
    // This is intended to be used to determine which region of the terrain
    // must be locked for the operation to be executed.
    const AABB& getAffectedRegion() const
    {
        return _affectedRegion;
    }
    
    // Performs the operation.
    virtual void perform(VoxelData &voxelData) = 0;
    
    // Serialize the operation.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(_affectedRegion);
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
