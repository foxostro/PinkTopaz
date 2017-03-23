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

namespace PinkTopaz::Terrain {
    
    // Accepts voxels and produces a triangle mesh for the specified isosurface.
    class Mesher
    {
    public:
        virtual ~Mesher() = default;
        
        // Returns a triangle mesh for the specified isosurface.
        virtual Renderer::StaticMesh
        extract(const VoxelData &voxels, float isosurface) = 0;
        
    protected:
        Mesher() = default;
    };
    
} // namespace PinkTopaz::Terrain

#endif /* Mesher_hpp */
