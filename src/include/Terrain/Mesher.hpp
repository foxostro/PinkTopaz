//
//  Mesher.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#ifndef Mesher_hpp
#define Mesher_hpp

#include "Terrain/Voxel.hpp"
#include "Grid/Array3D.hpp"
#include "Renderer/StaticMesh.hpp"

// Accepts voxels and produces a triangle mesh for the specified isosurface.
class Mesher
{
public:
    virtual ~Mesher() = default;
    
    // Returns a triangle mesh for the isosurface between value=0 and value=1.
    virtual StaticMesh
    extract(const Array3D<Voxel> &voxels,
            const AABB &region) = 0;
    
protected:
    Mesher() = default;
};

#endif /* Mesher_hpp */
