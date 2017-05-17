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
    
    // Returns a triangle mesh for the specified isosurface.
    virtual StaticMesh
    extract(const VoxelData &voxels, float isosurface) override;
    
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
    
    void polygonizeGridCell(StaticMesh &geometry,
                            const std::array<CubeVertex, NUM_CUBE_VERTS> &cube,
                            float isosurface);
};

#endif /* MesherMarchingCubes_hpp */