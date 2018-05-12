//
//  GridPoints.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/18/17.
//
//

#ifndef GridPoints_hpp
#define GridPoints_hpp

#include "GridIndexerRange.hpp"

// Return a Range object which can iterate over evenly spaced points in the
// specified region of the grid.
// Throws an exception if the region is not within this grid.
inline auto points(const GridIndexer &grid, const AABB &region)
{
    if constexpr (EnableVerboseBoundsChecking) {
        if (!grid.inbounds(region)) {
            throw OutOfBoundsException();
        }
    }
    
    const auto dim = grid.cellDimensions();
    const auto min = region.mins();
    const auto max = region.maxs();
    
    InclusiveIterator<glm::vec3> begin(min, max, min, dim, false);
    InclusiveIterator<glm::vec3> end(min, max, min, dim, true);
    
    return Range<InclusiveIterator<glm::vec3>>(begin, end);
}

#endif /* GridPoints_hpp */
