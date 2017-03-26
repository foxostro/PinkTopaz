//
//  Voxel.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef Voxel_hpp
#define Voxel_hpp

namespace Terrain {
    
    struct Voxel
    {
        float value;
        
        Voxel() : value(0.0f) {}
        Voxel(float v) : value(v) {}
    };

} // namespace Terrain

#endif /* Voxel_hpp */
