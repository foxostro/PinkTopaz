//
//  MesherNaiveSurfaceNets.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/5/17.
//
//

#ifndef MesherNaiveSurfaceNets_hpp
#define MesherNaiveSurfaceNets_hpp

#include "Terrain/Mesher.hpp"
#include <array>
#include <glm/glm.hpp>

// Accepts voxels and produces a triangle mesh for the specified isosurface.
// The extracted mesh uses a blocky style.
class MesherNaiveSurfaceNets : public Mesher
{
public:
    MesherNaiveSurfaceNets() = default;
    virtual ~MesherNaiveSurfaceNets() = default;
    
    // Returns a triangle mesh for the specified isosurface.
    virtual StaticMesh extract(const Array3D<Voxel> &voxels,
                               const AABB &region,
                               float isosurface) override;
    
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
    
    static constexpr size_t NUM_CUBE_EDGES = 12;
    static constexpr size_t NUM_CUBE_VERTS = 8;
    static constexpr size_t NUM_FACES = 6;
    
    // Smooth the vertex by pushing it down toward the isosurface.
    glm::vec3
    smoothVertex(const Array3D<Voxel> &voxels,
                 float isosurface,
                 const glm::vec3 &input);
    
    // Smooth each quad vertex by pushing them down toward the isosurface.
    std::array<glm::vec3, 4>
    smoothQuad(const Array3D<Voxel> &voxels,
               float isosurface,
               const std::array<glm::vec3, 4> &input);
    
    // Returns the four vertices for the quad which represents the specified
    // face of the specified cell, which has the shape of a rectangular prism.
    std::array<glm::vec3, 4>
    quadForFace(const AABB &cell, size_t face);
    
    // Returns the four texture coordinates for the quad which represents the
    // specified face of the specified cell.
    std::array<glm::vec2, 4>
    texCoordsForFace(const AABB &cell, size_t face);
    
    // Returns six vertices for the two triangles which constitute the specified
    // face of the specified cell, which has the shape of a rectangular prism.
    std::array<TerrainVertex, 6>
    verticesForFace(const Array3D<Voxel> &voxels,
                    float isosurface,
                    const AABB &cell,
                    size_t face);
    
    // Emits one face for the specified face of the specified cell. This face is
    // typically represented by six vertices contituting two triangles.
    void emitFace(StaticMesh &geometry,
                  const Array3D<Voxel> &voxels,
                  float isosurface,
                  const AABB &cell,
                  size_t face);
};

#endif /* MesherNaiveSurfaceNets_hpp */
