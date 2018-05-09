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
    float value;
    
    Voxel() : value(0.0f) {}
    Voxel(float v) : value(v) {}
    
    bool operator==(const Voxel &other) const
    {
        return other.value == value;
    }
    
    template<typename Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(value));
    }
};

#endif /* Voxel_hpp */
