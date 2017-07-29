//
//  TerrainDrawList.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainDrawList_hpp
#define TerrainDrawList_hpp

#include "RenderableStaticMesh.hpp"
#include "Terrain/Array3D.hpp"
#include "Terrain/TerrainMesh.hpp"
#include <experimental/optional>

// Meshes which are loaded and ready to be drawn.
// This is unlocked so take care to only access it from the render thread.
class TerrainDrawList
{
public:
    TerrainDrawList(const AABB &box, const glm::ivec3 &res);
    
    // Draw the meshes for the draw list.
    // Returns the list of meshe cells in the active region which are missing.
    std::vector<AABB>
    draw(const std::shared_ptr<CommandEncoder> &encoder,
         const Frustum &frustum,
         const AABB &activeRegion);
    
    // Update the draw list to include the specified GPU resources.
    void updateDrawList(const std::vector<TerrainMeshRef> &meshes);
    
private:
    // Draw a distinction between cells which have an empty mesh (i.e. air) and
    // cells for which we have not yet received a mesh at all.
    using MaybeMesh = typename std::experimental::optional<RenderableStaticMesh>;
    Array3D<MaybeMesh> _meshes;
};

#endif /* TerrainDrawList_hpp */
