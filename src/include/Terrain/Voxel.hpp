//
//  Voxel.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef Voxel_hpp
#define Voxel_hpp

struct Voxel
{
    float value;
    
    Voxel() : value(0.0f) {}
    Voxel(float v) : value(v) {}
    
    bool operator==(const Voxel &other) const
    {
        return other.value == value;
    }
};

#endif /* Voxel_hpp */
