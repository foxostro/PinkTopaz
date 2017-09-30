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
#include "Grid/SparseGrid.hpp"
#include "Terrain/TerrainMesh.hpp"
#include <shared_mutex>

// This is a draw list that is buffered so that updates on a background thread
// can proceed without interfering with the render thread.
class TerrainDrawList
{
public:
    TerrainDrawList(const AABB &box, const glm::ivec3 &res);
    
    // Draw the meshes for the draw list. This will attempt to swap the front
    // and back draw lists before drawing, but will not block on a lock to do
    // so. Returns the list of missing meshes in the active region.
    // Only call this from the render thread.
    std::vector<AABB>
    draw(const std::shared_ptr<CommandEncoder> &encoder,
         const Frustum &frustum,
         const AABB &activeRegion);
    
    // Update the back draw list to include the specified GPU resources.
    // Updates here will eventually show up in the front draw list once we're
    // able to swap the two lists in the draw method.
    void updateDrawList(const TerrainMesh &mesh);
    
    // We may evict meshes to keep the total mesh count under this limit.
    // Pass in at least the expected number of meshes in the working set.
    void setCountLimit(unsigned countLimit);
    
private:
    // Take this lock in exclusive mode to access `_back' with no other readers
    // or writers. Take this lock in shared mode to access `_back' with multiple
    // concurrent readers and writers.
    std::shared_mutex _lock;
    
    // Note that `_front' must only ever be accessed from the render thread.
    using MeshPtr = std::shared_ptr<RenderableStaticMesh>;
    SparseGrid<MeshPtr> _front, _back;
};

#endif /* TerrainDrawList_hpp */
