//
//  TerrainComponent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef TerrainComponent_hpp
#define TerrainComponent_hpp

#include "Terrain/Terrain.hpp"

// Gives terrain some representation in the ECS world.
struct TerrainComponent
{
    std::shared_ptr<Terrain> terrain;
};

#endif /* TerrainComponent_hpp */
