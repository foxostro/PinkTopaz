//
//  MesherBlocky.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/5/17.
//
//

#ifndef MesherBlocky_hpp
#define MesherBlocky_hpp

#include "Terrain/Mesher.hpp"
#include <array>

// Accepts voxels and produces a triangle mesh for the specified isosurface.
// The extracted mesh uses a blocky style.
class MesherBlocky : public Mesher
{
public:
    MesherBlocky() = default;
    virtual ~MesherBlocky() = default;
    
    // Returns a triangle mesh for the specified isosurface.
    virtual StaticMesh extract(const Array3D<Voxel> &voxels,
                               const AABB &region,
                               float isosurface) override;
    
protected:
    // Returns the four vertices for the quad which represents the specified
    // face of the specified cell, which has the shape of a rectangular prism.
    virtual std::array<glm::vec3, 4> quadForFace(const AABB &cell, size_t face);
    
private:
    static constexpr size_t NUM_FACES = 6;
    
    // Returns six vertices for the two triangles which constitute the specified
    // face of the specified cell, which has the shape of a rectangular prism.
    std::array<glm::vec3, 6> verticesForFace(const AABB &cell, size_t face);
    
    // Emits one vertex.
    void emitVertex(StaticMesh &geometry, const glm::vec3 &p, const glm::vec4 &color);
    
    // Emits one face for the specified face of the specified cell. This face is
    // typically represented by six vertices contituting two triangles.
    void emitFace(StaticMesh &geometry, const AABB &cell, size_t face);
};

#endif /* MesherBlocky_hpp */
