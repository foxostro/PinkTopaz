//
//  FrustumTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/24/17.
//
//

#include "catch.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Frustum.hpp"

using namespace glm;

TEST_CASE("Test Camera Plane Extraction", "[Frustum]") {
    // An identity view-projection matrix means the camera frustum is a cube
    // over [-1,+1]^3. The camera is at the origin pointing at the +Z direction.
    const mat4 viewProj(1.0f);
    Frustum frustum(viewProj);
    
    // Make sure we got the expected planes back.
    REQUIRE(frustum.planes()[Frustum::LeftPlane]   == normalize(Plane(+1.f,  0.f,  0.f, 1.f)));
    REQUIRE(frustum.planes()[Frustum::RightPlane]  == normalize(Plane(-1.f,  0.f,  0.f, 1.f)));
    REQUIRE(frustum.planes()[Frustum::BottomPlane] == normalize(Plane( 0.f, +1.f,  0.f, 1.f)));
    REQUIRE(frustum.planes()[Frustum::TopPlane]    == normalize(Plane( 0.f, -1.f,  0.f, 1.f)));
    REQUIRE(frustum.planes()[Frustum::NearPlane]   == normalize(Plane( 0.f,  0.f, +1.f, 1.f)));
    REQUIRE(frustum.planes()[Frustum::FarPlane]    == normalize(Plane( 0.f,  0.f, -1.f, 1.f)));
    
    // Test a few points we expect to be inside the frustum.
    REQUIRE(frustum.pointIsInside(vec3(0.0f, 0.0f,  0.2f)));
    REQUIRE(frustum.pointIsInside(vec3(0.0f, 0.0f, -0.2f)));
    REQUIRE(frustum.pointIsInside(vec3(0.0f, 0.0f,  1.f)));
    REQUIRE(frustum.pointIsInside(vec3(0.0f, 0.0f, -1.0f)));
    REQUIRE(frustum.pointIsInside(vec3(1.0f, 0.0f,  0.0f)));
    REQUIRE(frustum.pointIsInside(vec3(0.0f, 0.0f,  0.0f)));
    
    // Test a few points we expect to be outside the frustum.
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.0f, 0.0f, 100.0f)));
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.0f, 0.0f, 1.1f)));
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.0f, 0.0f, -1.1f)));
    
    // A box placed around the camera should be in the frustum.
    const AABB boxAroundCamera = { vec3(0.f), vec3(10.f) };
    REQUIRE(frustum.boxIsInside(boxAroundCamera));
    
    // A box placed directly in front of the camera should be in the frustum.
    const AABB boxInFront = { vec3(0.0f, 0.0f, 0.1f), vec3(0.1f) };
    REQUIRE(frustum.boxIsInside(boxInFront));
    
    // A box placed behind the camera should not be in the frustum
    const AABB boxInBack = { vec3(0.0f, 0.0f, -1.f), vec3(0.1f) };
    REQUIRE(frustum.boxIsInside(boxInBack));
    
    // A box which straddles the far plane should be in the frustum (partially)
    const AABB boxStraddleFar = { vec3(0.0f, 0.0f, 1.1f), vec3(0.2f) };
    REQUIRE(frustum.boxIsInside(boxStraddleFar));
}

TEST_CASE("Test Camera Plane Extraction 2", "[Frustum]") {
    // Setup the position and orientation of the camera.
    // The camera is at the origin and facing along the +Z direction.
    const mat4 view = lookAt(vec3(0.f, 0.f, 0.f),  // eye
                             vec3(0.f, 0.f, 1.f),  // center
                             vec3(0.f, 1.f, 0.f)); // up
    
    // Setup a standard perspective projection matrix.
    constexpr float fovy = pi<float>() * 0.25f;
    constexpr float znear = 1.0f;
    constexpr float zfar = 10.0f;
    constexpr float aspectRatio = 800.0f / 600.0f;
    const mat4 proj = perspective(fovy, aspectRatio, znear, zfar);
    
    // Get the frustum planes from the view-projection matrix.
    const mat4 viewProj = proj * view;
    Frustum frustum(viewProj);
    
    // The camera center should be in the frustum.
    REQUIRE(frustum.pointIsInside(vec3(0.f, 0.f, 2.f)));
    
    // As should any other point directly in front of the camera, within the near and far planes.
    REQUIRE(frustum.pointIsInside(vec3(0.f, 0.f, 5.f)));
    
    // A point closer than the near plane should not be in the frustum.
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.f, 0.f, 0.1f)));
    
    // A point past the far plane should not be in the frustum.
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.f, 0.f, 1000.0f)));
    
    // A point directly on the far plane should be in the frustum.
    REQUIRE(frustum.pointIsInside(vec3(0.f, 0.f, 10.f)));
    
    // A point directly on the near plane should be in the frustum.
    REQUIRE(frustum.pointIsInside(vec3(0.f, 0.f, 1.f)));
    
    // A point behind the camera not be in the frustum.
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.f, 0.f, -1.f)));
    
    // Test a few points far outside the sides of the frustum.
    REQUIRE_FALSE(frustum.pointIsInside(vec3(-100.f, 0.f, 2.f)));
    REQUIRE_FALSE(frustum.pointIsInside(vec3(+100.f, 0.f, 2.f)));
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.f, -100.f, 2.f)));
    REQUIRE_FALSE(frustum.pointIsInside(vec3(0.f, +100.f, 2.f)));
    
    // A box placed around the camera should be in the frustum.
    const AABB boxAroundCamera = { vec3(0.f), vec3(10.f) };
    REQUIRE(frustum.boxIsInside(boxAroundCamera));
    
    // A box placed directly in front of the camera should be in the frustum.
    const AABB boxInFront = { vec3(0.0f, 0.0f, 2.0f), vec3(1.f) };
    REQUIRE(frustum.boxIsInside(boxInFront));
}
