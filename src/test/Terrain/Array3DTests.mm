#import <XCTest/XCTest.h>
#import "Terrain/Array3D.hpp"

@interface Array3DTests : XCTestCase

@end

@implementation Array3DTests

- (void)testCtor {
	AABB box = {glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(2.0f, 2.0f, 2.0f)};
	glm::ivec3 res(4, 4, 4);
	Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssert(NO, "foo");
}

@end
