//
//  GridRaycast.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/18/17.
//
//

#ifndef GridRaycast_hpp
#define GridRaycast_hpp

#include "Ray.hpp"
#include <vector>
#include <glm/vec3.hpp>
#include <boost/coroutine2/all.hpp>


namespace GridRaycastRange {

// Iterate over cells which fall within the specified frustum.
static inline void
raycast(boost::coroutines2::coroutine<glm::vec3>::push_type &sink,
        const Ray &ray,
        size_t maxDepth)
{
    /* Implementation is based on:
     * "A Fast Voxel Traversal Algorithm for Ray Tracing"
     * John Amanatides, Andrew Woo
     * http://www.cse.yorku.ca/~amana/research/grid.pdf
     *
     * See also: http://www.xnawiki.com/index.php?title=Voxel_traversal
     */
    
    // NOTES:
    // * This code assumes that the ray's position and direction are in 'cell coordinates', which means
    //   that one unit equals one cell in all directions.
    // * When the ray doesn't start within the voxel grid, calculate the first position at which the
    //   ray could enter the grid. If it never enters the grid, there is nothing more to do here.
    // * Also, it is important to test when the ray exits the voxel grid when the grid isn't infinite.
    // * The Point3D structure is a simple structure having three integer fields (X, Y and Z).
    
    // The cell in which the ray starts.
    int x = (int)ray.origin.x;
    int y = (int)ray.origin.y;
    int z = (int)ray.origin.z;
    
    // Determine which way we go.
    int stepX = (ray.direction.x<0) ? -1 : (ray.direction.x==0) ? 0 : +1;
    int stepY = (ray.direction.y<0) ? -1 : (ray.direction.y==0) ? 0 : +1;
    int stepZ = (ray.direction.z<0) ? -1 : (ray.direction.z==0) ? 0 : +1;
    
    // Calculate cell boundaries. When the step (i.e. direction sign) is positive,
    // the next boundary is AFTER our current position, meaning that we have to add 1.
    // Otherwise, it is BEFORE our current position, in which case we add nothing.
    glm::ivec3 cellBoundary(x + (stepX > 0 ? 1 : 0),
                            y + (stepY > 0 ? 1 : 0),
                            z + (stepZ > 0 ? 1 : 0));
    
    // NOTE: For the following calculations, the result will be Single.PositiveInfinity
    // when ray.Direction.X, Y or Z equals zero, which is OK. However, when the left-hand
    // value of the division also equals zero, the result is Single.NaN, which is not OK.
    
    // Determine how far we can travel along the ray before we hit a voxel boundary.
    glm::vec3 tMax((cellBoundary.x - ray.origin.x) / ray.direction.x,    // Boundary is a plane on the YZ axis.
                   (cellBoundary.y - ray.origin.y) / ray.direction.y,    // Boundary is a plane on the XZ axis.
                   (cellBoundary.z - ray.origin.z) / ray.direction.z);   // Boundary is a plane on the XY axis.
    if (isnan(tMax.x)) { tMax.x = +INFINITY; }
    if (isnan(tMax.y)) { tMax.y = +INFINITY; }
    if (isnan(tMax.z)) { tMax.z = +INFINITY; }
    
    // Determine how far we must travel along the ray before we have crossed a gridcell.
    glm::vec3 tDelta(stepX / ray.direction.x,                    // Crossing the width of a cell.
                     stepY / ray.direction.y,                    // Crossing the height of a cell.
                     stepZ / ray.direction.z);                   // Crossing the depth of a cell.
    if (isnan(tDelta.x)) { tDelta.x = +INFINITY; }
    if (isnan(tDelta.y)) { tDelta.y = +INFINITY; }
    if (isnan(tDelta.z)) { tDelta.z = +INFINITY; }
    
    // For each step, determine which distance to the next voxel boundary is lowest (i.e.
    // which voxel boundary is nearest) and walk that way.
    for (size_t i = 0; i < maxDepth; ++i) {
        sink(glm::vec3(x, y, z));
        
        // Do the next step.
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            // tMax.X is the lowest, an YZ cell boundary plane is nearest.
            x += stepX;
            tMax.x += tDelta.x;
        } else if (tMax.y < tMax.z) {
            // tMax.Y is the lowest, an XZ cell boundary plane is nearest.
            y += stepY;
            tMax.y += tDelta.y;
        } else {
            // tMax.Z is the lowest, an XY cell boundary plane is nearest.
            z += stepZ;
            tMax.z += tDelta.z;
        }
    }
}

} // namespace GridRaycastRange


// Return a range to iterate over grid cells which fall on the specified ray.
// The ray is specified in world space coordinates.
static inline
boost::coroutines2::coroutine<glm::vec3>::pull_type
slice(const GridIndexer &grid,
      const Ray &ray,
      size_t maxDepth)
{
#ifdef EnableVerboseBoundsChecking
    if (!grid.inbounds(ray.origin)) {
        throw OutOfBoundsException();
    }
#endif
    
#if 0
    // TODO: Fix the conversion from world-space ray to cell-space direction.
    // Convert the world-space ray direction to a cell-space direction.
    const AABB box = grid.boundingBox();
    const glm::vec3 mins = box.mins();
    const glm::vec3 p = (ray.direction - mins) / (box.extent*2.0f);
    const glm::ivec3 res = grid.gridResolution();
    const glm::vec3 ccDir(glm::normalize(glm::vec3(p.x * res.x, p.y * res.y, p.z * res.z)));
    
    // Convert the world-space ray origin to cell-space.
    const glm::ivec3 iccOrigin = grid.cellCoordsAtPoint(ray.origin);
    const glm::vec3 ccOrigin(iccOrigin.x, iccOrigin.y, iccOrigin.z);
    
    return boost::coroutines2::coroutine<glm::vec3>::pull_type([&](boost::coroutines2::coroutine<glm::vec3>::push_type &sink){
        GridRaycastRange::raycast(sink, Ray(ccOrigin, ccDir), maxDepth);
    });
#else
    return boost::coroutines2::coroutine<glm::vec3>::pull_type([&](boost::coroutines2::coroutine<glm::vec3>::push_type &sink){
        GridRaycastRange::raycast(sink, ray, maxDepth);
    });
#endif
}


#endif /* GridRaycast_hpp */
