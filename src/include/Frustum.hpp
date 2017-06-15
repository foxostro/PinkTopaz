//
//  Frustum.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/14/17.
//
//

#ifndef Frustum_hpp
#define Frustum_hpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>
#include "AABB.hpp"
#include "Plane.hpp"
#include "math.hpp"

enum FrustumFace
{
    LeftPlane = 0,
    RightPlane,
    BottomPlane,
    TopPlane,
    NearPlane,
    FarPlane,
    NUM_PLANES
};

class Frustum
{
public:
    std::array<Plane, NUM_PLANES> planes;
    
    Frustum() = default;
    
    Frustum(const glm::mat4x4 &viewProj)
    {
        // Extract the view-frustum planes from the view-projection matrix.
        // See <https://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/>
        // for details on how to derive this trick.
        
        planes[LeftPlane]   = glm::row(viewProj, 3) + glm::row(viewProj, 0);
        planes[RightPlane]  = glm::row(viewProj, 3) - glm::row(viewProj, 0);
        planes[BottomPlane] = glm::row(viewProj, 3) + glm::row(viewProj, 1);
        planes[TopPlane]    = glm::row(viewProj, 3) - glm::row(viewProj, 1);
        planes[NearPlane]   = glm::row(viewProj, 3) + glm::row(viewProj, 2);
        planes[FarPlane]    = glm::row(viewProj, 3) - glm::row(viewProj, 2);
        
        // We need to normalize the planes in order for distance calculations to
        // be correct.
        planes[LeftPlane]   = glm::normalize(planes[LeftPlane]);
        planes[RightPlane]  = glm::normalize(planes[RightPlane]);
        planes[BottomPlane] = glm::normalize(planes[BottomPlane]);
        planes[TopPlane]    = glm::normalize(planes[TopPlane]);
        planes[NearPlane]   = glm::normalize(planes[NearPlane]);
        planes[FarPlane]    = glm::normalize(planes[FarPlane]);
    }
    
    bool inside(const AABB &box) const
    {
        for (const Plane &plane : planes) {
            // Since our axis-aligned bounding boxes are represented as a pair
            // {center, extent} we can manipulate the math a little to arrive at
            // the following test for whether the box is inside the plane.
            // See <https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/>
            // for details on the derivation.
            const float d = dot3(box.center, plane) + dot3(box.extent, glm::abs(plane));
            if (d < -plane.w) {
                return false;
            }
        }
        
        return true;
    }
    
    bool inside(const glm::vec3 &point) const
    {
        // If the point is in the negative halfspace of any plane then it is not
        // in the frustum. We permit points which lie exactly on a plane.
        // This test requires a normalized plane in order to work correctly.
        for (const auto &plane : planes) {
            const float d = signedDistance(plane, point);
            if (d < 0) {
                return false;
            }
        }
        
        return true;
    }
};

#endif /* Frustum_hpp */
