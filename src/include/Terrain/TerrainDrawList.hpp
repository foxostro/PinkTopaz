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

class TerrainDrawList
{
public:
    using MaybeTerrainMesh = typename std::experimental::optional<TerrainMesh>;
    
    TerrainDrawList(const AABB &box, const glm::ivec3 &res);
    
    // Draw the meshes.
    void draw(const std::shared_ptr<CommandEncoder> &encoder,
              const glm::mat4x4 &modelViewProjection);
    
    // Update the draw list to include the specified GPU resources.
    void updateDrawList(const TerrainMesh &mesh, const AABB &cell);
    
private:
    std::mutex _lockDrawList;
    Array3D<RenderableStaticMesh> _data;
    
    // Make this a member so as to avoid heap allocation every frame in draw().
    // This member is only ever used in draw() which is only ever used on the
    // render thread.
    std::vector<AABB> _cellsInFrustum;
};

#endif /* TerrainDrawList_hpp */
