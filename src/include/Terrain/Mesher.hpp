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
    
    // Produces meshes from blocks of voxels.
    class Mesher
    {
    public:
        Mesher() = default;
        ~Mesher() = default;
        
        // Produces a mesh from a block of voxels.
        Renderer::StaticMesh extract(const VoxelData &voxels, float isosurface);
        
    private:
        struct CubeVertex
        {
            const Voxel &voxel;
            const glm::vec3 &worldPos;
            
            CubeVertex(const VoxelData &voxels, const glm::vec3 &w)
            : voxel(voxels.get(w)), worldPos(w)
            {}
        };
        
        static constexpr int NUM_CUBE_EDGES = 12;
        static constexpr int NUM_CUBE_VERTS = 8;
        
        void polygonizeGridCell(Renderer::StaticMesh &geometry,
                                const std::array<CubeVertex, NUM_CUBE_VERTS> &cube,
                                float isosurface);
    };
    
} // namespace PinkTopaz::Terrain

#endif /* Mesher_hpp */
