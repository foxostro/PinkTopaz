//
//  Octree.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/15/17.
//
//

#include "Octree.hpp"
#include "Frustum.hpp"
#include "math.hpp" // for ilog2() and isPowerOfTwo()

Octree::Octree(const AABB &box, const glm::ivec3 &res)
 : _depthOfLeaves(ilog2(res.x)), _world(box)
{
    assert((res.x == res.y) && (res.x == res.z));
    assert(isPowerOfTwo(res.x));
}

void Octree::select(std::vector<AABB> &result,
                    size_t depth,
                    const AABB &box,
                    const Frustum &frustum) const
{
    if (frustum.boxIsInside(box)) {
        if (depth == _depthOfLeaves) {
            result.push_back(box);
        } else {
            for (auto &octant : box.octants()) {
                select(result, depth+1, octant, frustum);
            }
        }
    }
}
