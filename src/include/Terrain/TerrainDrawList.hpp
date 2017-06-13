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
    typedef typename std::experimental::optional<TerrainMesh> MaybeTerrainMesh;
    
    TerrainDrawList(const AABB &box, const glm::ivec3 &res);
    
    // Draw the meshes.
    void draw(const std::shared_ptr<CommandEncoder> &encoder);
    
    // If we can get a hold of the underlying GPU resources then add them to the
    // draw list.
    void tryUpdateDrawList(const MaybeTerrainMesh &maybeTerrainMesh,
                           const AABB &cell);
    
private:
    std::mutex _lockDrawList;
    Array3D<RenderableStaticMesh> _data;
};

#endif /* TerrainDrawList_hpp */
