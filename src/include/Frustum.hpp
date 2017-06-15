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

class Frustum
{
public:
    enum
    {
        LeftPlane = 0,
        RightPlane,
        BottomPlane,
        TopPlane,
        NearPlane,
        FarPlane,
        NumPlanes
    };
    
    Frustum() = default;
    
    Frustum(const glm::mat4x4 &viewProj)
    {
        // Extract the view-frustum planes from the view-projection matrix.
        // See <https://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/>
        // for details on how to derive this trick.
        
        _planes[LeftPlane]   = glm::row(viewProj, 3) + glm::row(viewProj, 0);
        _planes[RightPlane]  = glm::row(viewProj, 3) - glm::row(viewProj, 0);
        _planes[BottomPlane] = glm::row(viewProj, 3) + glm::row(viewProj, 1);
        _planes[TopPlane]    = glm::row(viewProj, 3) - glm::row(viewProj, 1);
        _planes[NearPlane]   = glm::row(viewProj, 3) + glm::row(viewProj, 2);
        _planes[FarPlane]    = glm::row(viewProj, 3) - glm::row(viewProj, 2);
        
        // We need to normalize the planes in order for distance calculations to
        // be correct.
        _planes[LeftPlane]   = glm::normalize(_planes[LeftPlane]);
        _planes[RightPlane]  = glm::normalize(_planes[RightPlane]);
        _planes[BottomPlane] = glm::normalize(_planes[BottomPlane]);
        _planes[TopPlane]    = glm::normalize(_planes[TopPlane]);
        _planes[NearPlane]   = glm::normalize(_planes[NearPlane]);
        _planes[FarPlane]    = glm::normalize(_planes[FarPlane]);
        
        // Precompute the absolute value of the planes. We use this in inside().
        _absPlanes[LeftPlane]   = glm::abs(_planes[LeftPlane]);
        _absPlanes[RightPlane]  = glm::abs(_planes[RightPlane]);
        _absPlanes[BottomPlane] = glm::abs(_planes[BottomPlane]);
        _absPlanes[TopPlane]    = glm::abs(_planes[TopPlane]);
        _absPlanes[NearPlane]   = glm::abs(_planes[NearPlane]);
        _absPlanes[FarPlane]    = glm::abs(_planes[FarPlane]);
    }
    
    // Determine whether an axis-aligned bbox resides within the frustum.
    bool boxIsInside(const AABB &box) const
    {
        for (size_t i = 0; i < NumPlanes; ++i) {
            const auto &plane = _planes[i];
            const auto &absPl = _absPlanes[i];
            
            // Since our axis-aligned bounding boxes are represented as a pair
            // {center, extent} we can manipulate the math a little to arrive at
            // the following test for whether the box is inside the plane.
            // See <https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/>
            // for details on the derivation.
            const float d = dot3(box.center, plane) + dot3(box.extent, absPl);
            if (d < -plane.w) {
                return false;
            }
        }
        
        return true;
    }
    
    // Determine whether a point resides within the frustum.
    bool pointIsInside(const glm::vec3 &point) const
    {
        // If the point is in the negative halfspace of any plane then it is not
        // in the frustum. We permit points which lie exactly on a plane.
        // This test requires a normalized plane in order to work correctly.
        for (const auto &plane : _planes) {
            const float d = signedDistance(plane, point);
            if (d < 0) {
                return false;
            }
        }
        
        return true;
    }
    
    // Get the planes.
    inline const auto& planes() const
    {
        return _planes;
    }
    
private:
    std::array<Plane, NumPlanes> _planes;
    std::array<Plane, NumPlanes> _absPlanes;
};

#endif /* Frustum_hpp */
