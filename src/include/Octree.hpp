//
//  Octree.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/14/17.
//
//

#ifndef Octree_hpp
#define Octree_hpp

#include "Frustum.hpp"

// Octree drastically reduces the number of boxes we have to test when culling
// against the camera view-frustum. It's not really a tree, but it does divide
// space into nested octants, so there.
class Octree
{
public:
    Octree(const AABB &box, const glm::ivec3 &res);
    
    // Return in `result' the list of cells in the view-frustum. 
    inline void select(std::vector<AABB> &result, const Frustum &frustum) const
    {
        result.clear();
        select(result, 0, _world, frustum);
    }
    
private:
    const size_t _depthOfLeaves;
    const AABB _world;
    
    void select(std::vector<AABB> &result,
                size_t depth,
                const AABB &box,
                const Frustum &frustum) const;
};

#endif /* Octree_hpp */
