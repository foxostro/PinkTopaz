//
//  Voxel.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/13/16.
//
//

#ifndef Voxel_hpp
#define Voxel_hpp

namespace PinkTopaz::Terrain {
    
    struct Voxel
    {
        float value;
        
        Voxel() : value(0.0f) {}
    };

} // namespace PinkTopaz::Terrain

#endif /* Voxel_hpp */
