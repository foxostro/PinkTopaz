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

// Range over grid cells which fall within a specified frustum.
class FrustumRange
{
public:
    FrustumRange() = delete;
    
    FrustumRange(const Frustum &frustum,
                 const AABB &sliceBoundingBox,
                 const GridIndexer &grid)
    {
        const auto &res = grid.gridResolution();
        const auto &bbox = grid.boundingBox();
        assert((res.x == res.y) && (res.x == res.z));
        assert(isPowerOfTwo(res.x));
        forEachCell(0, ilog2(res.x), bbox, frustum, sliceBoundingBox,
                    _cells, grid);
    }
    
    inline auto begin() const
    {
        return _cells.begin();
    }
    
    inline auto end() const
    {
        return _cells.end();
    }
    
private:
    std::vector<glm::ivec3> _cells;
    
    // Iterate over cells which fall within the specified frustum.
    void forEachCell(size_t depth,
                     size_t depthOfLeaves,
                     const AABB &box,
                     const Frustum &frustum,
                     const AABB &sliceBoundingBox,
                     std::vector<glm::ivec3> &cells,
                     const GridIndexer &grid) const
    {
        if (frustum.boxIsInside(box) &&
            frustum.boxIsInside(sliceBoundingBox)) {
            
            if (depth == depthOfLeaves) {
                cells.push_back(grid.cellCoordsAtPoint(box.center));
            } else {
                for (auto &octant : box.octants()) {
                    forEachCell(depth+1, depthOfLeaves, octant, frustum,
                                sliceBoundingBox, cells, grid);
                }
            }
        }
    }
};

// Return a range object which can iterate over a frustum within the grid.
static inline auto slice(const GridIndexer &grid,
                         const Frustum &frustum,
                         const AABB &sliceBoundingBox)
{
    return FrustumRange(frustum, sliceBoundingBox, grid);
}

#endif /* FrustumRange_hpp */
