//
//  TerrainMeshGrid.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/27/17.
//
//

#ifndef TerrainMeshGrid_hpp
#define TerrainMeshGrid_hpp

#include "Terrain/ConcurrentGridMutable.hpp"
#include "Terrain/TerrainMesh.hpp"
#include "optional.hpp"

using MaybeTerrainMesh = typename optional<TerrainMesh>;

class TerrainMeshGrid : public ConcurrentGridMutable<MaybeTerrainMesh>
{
public:
    // Constructor. The space is divided into a grid of cells where each cell
    // is associated with a data element. The space is also divided into a grid
    // of cells where each cell has a lock to protect the data grid from
    // concurrent access. Though, the two grids may have a different resolution
    // such that each lock protects a region of space instead of a single data
    // element.
    // array -- The array to wrap.
    // box -- The region of space for which this grid is valid.
    // lockGridResDivisor -- The resolution of the lock grid is determined by
    //                       dividing the array's grid resolution by this.
    TerrainMeshGrid(std::unique_ptr<GridMutable<MaybeTerrainMesh>> &&array,
                    unsigned lockGridResDivisor);
    
    // Perform an atomic transaction as a "reader" with read-only access to the
    // underlying data in the specified region.
    // region -- The region we will be reading from.
    // onPresent -- Execute this for each item that is present, or could be locked.
    void readerTransactionTry(const AABB &region,
                              const std::function<void(const TerrainMesh &terrainMesh)> &onPresent) const;
};

#endif /* TerrainMeshGrid_hpp */
