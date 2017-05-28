//
//  Mesher.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#ifndef Mesher_hpp
#define Mesher_hpp

#include "VoxelData.hpp"
#include "Renderer/StaticMesh.hpp"

// Accepts voxels and produces a triangle mesh for the specified isosurface.
class Mesher
{
public:
    virtual ~Mesher() = default;
    
    // Returns a triangle mesh for the specified isosurface.
    virtual StaticMesh
    extract(const Array3D<Voxel> &voxels,
            const AABB &region,
            float isosurface) = 0;
    
protected:
    Mesher() = default;
};

#endif /* Mesher_hpp */
