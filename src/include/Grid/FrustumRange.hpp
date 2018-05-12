//
//  FrustumRange.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/18/17.
//
//

#ifndef FrustumRange_hpp
#define FrustumRange_hpp

#include "GridIndexer.hpp"
#include "Frustum.hpp"

#include <boost/coroutine2/all.hpp>

namespace FrustumRange {

// Iterate over cells which fall within the specified frustum.
static inline void forEachCell(boost::coroutines2::coroutine<glm::ivec3>::push_type &sink,
                               size_t depth,
                               size_t depthOfLeaves,
                               const AABB &box,
                               const Frustum &frustum,
                               const AABB &sliceBoundingBox,
                               const GridIndexer &grid)
{
    if (frustum.boxIsInside(box) &&
        frustum.boxIsInside(sliceBoundingBox)) {
        
        if (depth == depthOfLeaves) {
            glm::ivec3 point = grid.cellCoordsAtPoint(box.center);
            sink(point);
        } else {
            for (auto &octant : box.octants()) {
                forEachCell(sink, depth+1, depthOfLeaves, octant, frustum,
                            sliceBoundingBox, grid);
            }
        }
    }
}
    
} // namespace FrustumRange

// Return a range object which can iterate over a frustum within the grid.
static inline
boost::coroutines2::coroutine<glm::ivec3>::pull_type
slice(const GridIndexer &grid,
      const Frustum &frustum,
      const AABB &sliceBoundingBox)
{
    const auto res = grid.gridResolution();
    const auto bbox = grid.boundingBox();
    assert((res.x == res.y) && (res.x == res.z));
    assert(isPowerOfTwo(res.x));
    return boost::coroutines2::coroutine<glm::ivec3>::pull_type([&](boost::coroutines2::coroutine<glm::ivec3>::push_type &sink){
        FrustumRange::forEachCell(sink, 0, ilog2(res.x), bbox, frustum,
                                  sliceBoundingBox, grid);
    });
}

#endif /* FrustumRange_hpp */
