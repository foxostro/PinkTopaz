//
//  TerrainOperationEditPoint.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#ifndef TerrainOperationEditPoint_hpp
#define TerrainOperationEditPoint_hpp

#include "TerrainOperation.hpp"
#include "CerealGLM.hpp"

// An operation which edits a single voxel block.
// This is useful for edits made with the mouse cursor ala Minecraft.
class TerrainOperationEditPoint : public TerrainOperation
{
public:
    // Default destructor.
    virtual ~TerrainOperationEditPoint() = default;
    
    // Default constructor
    TerrainOperationEditPoint() = default;
    
    // Constructor.
    TerrainOperationEditPoint(glm::vec3 location, Voxel newValue);
    
    // Performs the operation.
    void perform(Array3D<Voxel> &voxelData) override;
    
    // Serialize the operation.
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(cereal::base_class<TerrainOperation>(this),
                cereal::make_nvp("location", _location),
                cereal::make_nvp("newValue", _newValue));
    }
    
private:
    glm::vec3 _location;
    Voxel _newValue;
};

CEREAL_REGISTER_TYPE(TerrainOperationEditPoint);

#endif /* TerrainOperationEditPoint_hpp */
