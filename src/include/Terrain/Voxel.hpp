//
//  Voxel.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef Voxel_hpp
#define Voxel_hpp

#include <cereal/archives/binary.hpp>

struct Voxel
{
    uint32_t value:1;
    uint32_t sunLight:4;
    uint32_t torchLight:4;
    
    Voxel() : value(0), sunLight(0), torchLight(0) {}
    explicit Voxel(bool v) : value(v ? 1 : 0), sunLight(0), torchLight(0) {}
    
    bool operator==(const Voxel &other) const
    {
        return other.value == value;
    }
    
    template<typename Archive>
    void serialize(Archive &archive)
    {
        unsigned valueCopy = value;
        unsigned sunLightCopy = sunLight;
        unsigned torchLightCopy = torchLight;
        
        archive(valueCopy, sunLightCopy, torchLightCopy);
        
        value = valueCopy;
        sunLight = sunLightCopy;
        torchLight = torchLightCopy;
    }
};

static_assert(sizeof(Voxel) == 4, "Voxel is expected to be four bytes.");

#endif /* Voxel_hpp */
