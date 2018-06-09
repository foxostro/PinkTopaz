//
//  TerrainMeshGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/27/17.
//
//

#ifndef TerrainMeshGrid_hpp
#define TerrainMeshGrid_hpp

#include "Grid/LimitedConcurrentSparseGrid.hpp"
#include "Terrain/TerrainMesh.hpp"
#include <memory>

// TerrainMeshGrid is a grid of TerrainMesh objects. In practice, this is a grid
// covering the entire world and represents the polygonal mesh representation
// of the voxel terrain, broken up into regular chunks.
//
// * We use SparseGrid for this because we need to avoid spending large amounts
//   of memory on TerrainMesh objects for regions of world that have never been
//   seen, and may never be seen.
// * We wrap those meshes in shared_ptr so that we can discard objects from the
//   sparse grid as needed to stay under a specified memory limit.
//   The use of shared_ptr ensures that references we've vended out in the past
//   remain live until they no longer in use.
using TerrainMeshGrid = LimitedConcurrentSparseGrid<std::shared_ptr<TerrainMesh>>;

#endif /* TerrainMeshGrid_hpp */
