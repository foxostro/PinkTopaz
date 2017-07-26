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
#include <shared_mutex>

// This is a draw list that is buffered so that updates on a background thread
// can proceed without interfering with the render thread. It maintains two draw
// lists and swaps between them. When the draw method is called on the render
// thread, we will attempt to swap the two draw lists without blocking.
class TerrainDrawList
{
public:
    TerrainDrawList(const AABB &box, const glm::ivec3 &res);
    
    // Draw the meshes for the draw list. This will attempt to swap the front
    // and back draw lists before drawing, but will not block on a lock to do
    // so.
    void draw(const std::shared_ptr<CommandEncoder> &encoder,
              const Frustum &frustum);
    
    // Update the back draw list to include the specified GPU resources.
    // Updates here will eventually show up in the front draw list once we're
    // able to swap the two lists in the draw method.
    void updateDrawList(const TerrainMesh &mesh, const AABB &cell);
    
private:
    std::shared_mutex _lock;
    std::unique_ptr<ConcurrentGridMutable<RenderableStaticMesh>> _front, _back;
};

#endif /* TerrainDrawList_hpp */
