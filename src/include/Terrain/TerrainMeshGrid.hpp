//
//  TerrainMeshGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/27/17.
//
//

#ifndef TerrainMeshGrid_hpp
#define TerrainMeshGrid_hpp

#include "Grid/ConcurrentGridMutable.hpp"
#include "Terrain/TerrainMesh.hpp"
#include <boost/optional.hpp>

using MaybeTerrainMesh = boost::optional<TerrainMesh>;
using TerrainMeshGrid = ConcurrentGridMutable<MaybeTerrainMesh>;

#endif /* TerrainMeshGrid_hpp */
