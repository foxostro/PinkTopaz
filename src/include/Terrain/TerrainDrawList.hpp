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
#include <experimental/optional>

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
    
private:
    // Updates the front draw list to include new information from the back one.
    // Only call this on the render thread.
    void updateFrontList(const AABB &activeRegion);
    
    // Take this lock in exclusive mode to access `_back' with no other readers
    // or writers. Take this lock in shared mode to access `_back' with multiple
    // concurrent readers and writers.
    std::shared_mutex _lock;
    
    // Draw a distinction between cells which have an empty mesh (i.e. air) and
    // cells for which we have not yet received a mesh at all.
    // Note that `_front' must only ever be accessed from the render thread.
    using MaybeMesh = typename std::experimental::optional<RenderableStaticMesh>;
    Array3D<MaybeMesh> _front;
    ConcurrentGridMutable<MaybeMesh> _back;
};

#endif /* TerrainDrawList_hpp */
