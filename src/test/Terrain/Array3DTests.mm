#import <XCTest/XCTest.h>
#import "Terrain/Array3D.hpp"
#import "Exception.hpp"

using glm::vec3;
using glm::ivec3;

@interface Array3DTests : XCTestCase

@end

@implementation Array3DTests

- (void)testCellCoordsAtPoint {
	AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
	ivec3 res(4, 4, 4);
	Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    glm::ivec3 a, b;
    
    // Adding the cell dimensions to the cell's minimum-corner takes us to the
    // neighboring cell. The range is not inclusive.
    a = myArray.cellCoordsAtPoint(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPoint(myArray.cellDimensions());
    XCTAssertEqual(a + ivec3(1, 1, 1), b);
    
    // Two points which reside in the same cell result in the same cell coords.
    a = myArray.cellCoordsAtPoint(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPoint(myArray.cellDimensions() * 0.5f);
    XCTAssertEqual(a, b);
}

- (void)testCellCenterAtPoint {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    // Test a point on the minimum edge of a cell.
    XCTAssertEqual(myArray.cellCenterAtPoint(vec3(0.0f, 0.0f, 0.0f)),
                   vec3(0.5f, 0.5f, 0.5f));
    
    // Test some arbitrary point within the cell.
    XCTAssertEqual(myArray.cellCenterAtPoint(vec3(0.7f, 0.7f, 0.7f)),
                   vec3(0.5f, 0.5f, 0.5f));
    
    // Test a point on the maximum edge of the cell. This range of the cell is
    // exclusive so this edge is actually in a different cell.
    XCTAssertNotEqual(myArray.cellCenterAtPoint(vec3(1.0f, 1.0f, 1.0f)),
                      vec3(0.5f, 0.5f, 0.5f));
    
    // Make sure the method throws an exception when given a point which is not
    // in the valid space of the grid.
    try {
        myArray.cellCenterAtPoint(vec3(-10.0f, 0.0f, 0.0f));
        XCTFail("We expected an exception before reaching this line.");
    } catch(const OutOfBoundsException &e) {
        // Swallow the exception without failing the test.
    }
    
    // Repeat the above scenario using the maximum-corner of the entire grid.
    // Like individual cells, this is exclusive and should not be considered to
    // be a part of the valid space of the grid.
    try {
        myArray.cellCenterAtPoint(vec3(4.0f, 4.0f, 4.0f));
        XCTFail("We expected an exception before reaching this line.");
    } catch(const OutOfBoundsException &e) {
        // Swallow the exception without failing the test.
    }
}

@end
