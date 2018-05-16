//
//  MesherMarchingCubes.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#ifndef MesherMarchingCubes_hpp
#define MesherMarchingCubes_hpp

#include "Terrain/Mesher.hpp"

// Accepts voxels and produces a triangle mesh for the specified isosurface.
class MesherMarchingCubes : public Mesher
{
public:
    MesherMarchingCubes() = default;
    virtual ~MesherMarchingCubes() = default;
    
    // Returns a triangle mesh for the isosurface between value=0 and value=1.
    virtual StaticMesh extract(const Array3D<Voxel> &voxels,
                               const AABB &region) override;
    
private:
    // For marching cubes, we sample a cube where each vertex is a voxel in the
    // voxel grid.
    struct CubeVertex
    {
        // Reference to the voxel associated with this vertex of the cube.
        const Voxel &voxel;
        const glm::vec3 &worldPos;
        const glm::vec3 cellRelativeVertexPos;
        
        CubeVertex(const Voxel &voxel, const glm::vec3 &w, const glm::vec3 &c)
         : voxel(voxel), worldPos(w), cellRelativeVertexPos(c)
        {}
    };
    
    static constexpr int NUM_CUBE_EDGES = 12;
    static constexpr int NUM_CUBE_VERTS = 8;
    
    void polygonizeGridCell(StaticMesh &geometry,
                            const std::array<CubeVertex, NUM_CUBE_VERTS> &cube,
                            float isosurface);
};

#endif /* MesherMarchingCubes_hpp */
