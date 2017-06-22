#import <XCTest/XCTest.h>
#import "Terrain/Array3D.hpp"
#import "Exception.hpp"
#import "SDL.h"

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
    
    ivec3 a, b;
    
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

- (void)testCellCoordsAtPointRoundUp {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    ivec3 a, b;
    
    // Adding the cell dimensions to the cell's minimum-corner takes us to the
    // neighboring cell. The range is not inclusive.
    a = myArray.cellCoordsAtPointRoundUp(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPointRoundUp(myArray.cellDimensions());
    XCTAssertEqual(a + ivec3(1, 1, 1), b);
    
    // We round up each coordinate so a cell at <0.5, 0.5, 0.5> will return the
    // coordinates for the cell at <1, 1, 1> and not <0, 0, 0> as we would see
    // with `cellCoordsAtPoint'.
    a = ivec3(1, 1, 1);
    b = myArray.cellCoordsAtPointRoundUp(myArray.cellDimensions() * 0.5f);
    XCTAssertEqual(a, b);
}

- (void)testCellCenterAtCellCoords {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    XCTAssertEqual(myArray.cellCenterAtCellCoords(ivec3(0, 0, 0)),
                   vec3(0.5f, 0.5f, 0.5f));
    
    XCTAssertEqual(myArray.cellCenterAtCellCoords(ivec3(2, 2, 2)),
                   vec3(2.5f, 2.5f, 2.5f));
    
    XCTAssertEqual(myArray.cellCenterAtCellCoords(ivec3(3, 3, 3)),
                   vec3(3.5f, 3.5f, 3.5f));
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
}

- (void) testCellAtPoint {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    AABB cell = myArray.cellAtPoint(vec3(0.1f, 0.1f, 0.1f));
    XCTAssertEqual(cell.center, vec3(0.5f, 0.5f, 0.5f));
    XCTAssertEqual(cell.extent, vec3(0.5f, 0.5f, 0.5f));
    
    cell = myArray.cellAtPoint(vec3(1.0f, 0.5f, 0.5f));
    XCTAssertEqual(cell.center, vec3(1.5f, 0.5f, 0.5f));
    XCTAssertEqual(cell.extent, vec3(0.5f, 0.5f, 0.5f));
    
    cell = myArray.cellAtPoint(vec3(1.0f, 1.0f, 1.0f));
    XCTAssertEqual(cell.center, vec3(1.5f, 1.5f, 1.5f));
    XCTAssertEqual(cell.extent, vec3(0.5f, 0.5f, 0.5f));
}

- (void) testCountCellsInRegion {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    // If we pass in the exact bounding box then we should get the original grid
    // resolution back again.
    XCTAssertEqual(res, myArray.countCellsInRegion(box));
    
    ivec3 c;
    
    // A region covering exactly one cell.
    const AABB oneCell = {vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)};
    c = myArray.countCellsInRegion(oneCell);
    XCTAssertEqual(ivec3(1, 1, 1), c);
    
    // The maximum-corner of a region is exclusive and is not part of the region
    // which means that a zero-sized region contains zero cells.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    c = myArray.countCellsInRegion(zeroBox);
    XCTAssertEqual(ivec3(0, 0, 0), c);
    
    // Currently, countCellsInRegion() returns <0,0,0> when the min and max
    // corners of the box are in the same cell. I'm not sure this is correct
    // behavior, but it is the current behavior and the test verifies that this
    // behavior does not change unintentionally.
    const AABB tenthBox1 = {vec3(0.1f, 0.1f, 0.1f), vec3(0.1f, 0.1f, 0.1f)};
    c = myArray.countCellsInRegion(tenthBox1);
    XCTAssertEqual(ivec3(0, 0, 0), c);
    
    // Currently, countCellsInRegion() returns <1,1,1> when the min and max
    // corners of the box are in adjacent cells. I'm not sure this is correct
    // behavior, but it is the current behavior and the test verifies that this
    // behavior does not change unintentionally.
    const AABB tenthBox2 = {vec3(1.0f, 1.0f, 1.0f), vec3(0.1f, 0.1f, 0.1f)};
    c = myArray.countCellsInRegion(tenthBox2);
    XCTAssertEqual(ivec3(1, 1, 1), c);
}

- (void)testSnapRegionToCellBoundaries {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    AABB a, b;
    
    // If we pass in the exact bounding box then we should get the original box
    // since it's already snapped to cell boundaries.
    a = box;
    b = myArray.snapRegionToCellBoundaries(box);
    XCTAssertEqual(a, b);
    
    // A zero size box is snapped to the center of the cell that its center
    // falls within.
    const AABB zeroBox1 = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    a = {vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 0.0f, 0.0f)};
    b = myArray.snapRegionToCellBoundaries(zeroBox1);
    XCTAssertEqual(a, b);
    
    // Try a box that includes only one cell.
    const AABB oneCellBox = {vec3(0.5, 0.5, 0.5f), vec3(0.1f, 0.1f, 0.1f)};
    a = myArray.cellAtPoint(oneCellBox.center);
    b = myArray.snapRegionToCellBoundaries(oneCellBox);
    XCTAssertEqual(a, b);
    
    // Try a box that covers several cells and which does not already cover
    // exact cell dimensions.
    const AABB box2 = {vec3(0.9f, 0.9f, 0.9f), vec3(1.0f, 1.0f, 1.0f)};
    a = myArray.snapRegionToCellBoundaries(box2);
    b = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    XCTAssertEqual(a, b);
}

- (void) testInbounds {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    // The grid bounding box is obviously within the valid space of the grid.
    XCTAssert(myArray.inbounds(box));
    
    // A zero-sized region within the valid space of the grid is in-bounds much
    // the same as a point would be.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    XCTAssert(myArray.inbounds(zeroBox));
    
    // A small box within the grid is in-bounds.
    const AABB tenthBox = {vec3(0.1f, 0.1f, 0.1f), vec3(0.1f, 0.1f, 0.1f)};
    XCTAssert(myArray.inbounds(tenthBox));
    
    // A region totally outside the grid bounding box is not in-bounds.
    const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
    XCTAssertFalse(myArray.inbounds(negCell));
    
    // A region which intersects the bounding box but is not wholly within the
    // box is not considered in-bounds.
    const AABB pastMin = {vec3(0.5f, 0.5f, 0.5f), vec3(0.7f, 0.7f, 0.7f)};
    XCTAssertFalse(myArray.inbounds(pastMin));
}

- (void) testForEachCell {
    AABB box = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    ivec3 res(2, 2, 2);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    const std::vector<AABB> desiredCells = {
        {vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 1.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 1.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 0.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 1.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 0.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 1.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
    };
    
    std::vector<AABB> actualCells;
    
    myArray.forEachCell(box, [&](const AABB &cell, Morton3 index, const int &value){
        actualCells.push_back(cell);
    });
    
    // Make sure we got all the cells we intended to.
    XCTAssertEqual(desiredCells, actualCells);
    
    // Throws an exception when the region is not in-bounds.
    try {
        const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
        myArray.forEachCell(negCell, [&](const AABB &cell, Morton3 index, const int &value){
            SDL_Log("{(%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)}",
                    cell.center.x, cell.center.y, cell.center.z,
                    cell.extent.x, cell.extent.y, cell.extent.z);
        });
        XCTFail("We expected an exception before reaching this line.");
    } catch(const OutOfBoundsException &e) {
        // Swallow the exception without failing the test.
    }
    
    // If the region is a zero-sized box then we should iterate over no cells.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    actualCells.clear();
    myArray.forEachCell(zeroBox, [&](const AABB &cell, Morton3 index, const int &value){
        XCTFail("We expected to iterate over zero cells in this case.");
    });
}

- (void) testForPointsInGrid {
    AABB box = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    ivec3 res(2, 2, 2);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    const std::vector<vec3> desiredPoints = {
        vec3(0.f, 0.f, 0.f),
        vec3(0.f, 1.f, 0.f),
        vec3(0.f, 2.f, 0.f),
        vec3(1.f, 0.f, 0.f),
        vec3(1.f, 1.f, 0.f),
        vec3(1.f, 2.f, 0.f),
        vec3(2.f, 0.f, 0.f),
        vec3(2.f, 1.f, 0.f),
        vec3(2.f, 2.f, 0.f),
        vec3(0.f, 0.f, 1.f),
        vec3(0.f, 1.f, 1.f),
        vec3(0.f, 2.f, 1.f),
        vec3(1.f, 0.f, 1.f),
        vec3(1.f, 1.f, 1.f),
        vec3(1.f, 2.f, 1.f),
        vec3(2.f, 0.f, 1.f),
        vec3(2.f, 1.f, 1.f),
        vec3(2.f, 2.f, 1.f),
        vec3(0.f, 0.f, 2.f),
        vec3(0.f, 1.f, 2.f),
        vec3(0.f, 2.f, 2.f),
        vec3(1.f, 0.f, 2.f),
        vec3(1.f, 1.f, 2.f),
        vec3(1.f, 2.f, 2.f),
        vec3(2.f, 0.f, 2.f),
        vec3(2.f, 1.f, 2.f),
        vec3(2.f, 2.f, 2.f),
    };
    
    std::vector<vec3> actualPoints;
    
    myArray.forPointsInGrid(box, [&](const vec3 &point){
        actualPoints.push_back(point);
    });
    
    // Make sure we got all the points we intended to.
    XCTAssertEqual(desiredPoints, actualPoints);
    
    // Throws an exception when the region is not in-bounds.
    try {
        const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
        myArray.forPointsInGrid(negCell, [&](const vec3 &point){
            SDL_Log("(%.2f, %.2f, %.2f)", point.x, point.y, point.z);
        });
        XCTFail("We expected an exception before reaching this line.");
    } catch(const OutOfBoundsException &e) {
        // Swallow the exception without failing the test.
    }
    
    // If the region is a zero-sized box then we should iterate over one point.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    actualPoints.clear();
    myArray.forPointsInGrid(zeroBox, [&](const vec3 &point){
        actualPoints.push_back(point);
    });
    XCTAssertEqual(1, actualPoints.size());
}

- (void) testMutableForEachCell {
    AABB box = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    ivec3 res(2, 2, 2);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    const std::vector<AABB> desiredCells = {
        {vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 1.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 1.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 0.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(0.5f, 1.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 0.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
        {vec3(1.5f, 1.5f, 1.5f), vec3(0.5f, 0.5f, 0.5f)},
    };
    
    std::vector<AABB> actualCells;
    
    myArray.mutableForEachCell(box, [&](const AABB &cell, Morton3 index, int &value){
        actualCells.push_back(cell);
        value = 42;
    });
    
    // Make sure we touched all the cells we intended to.
    XCTAssertEqual(desiredCells, actualCells);
    
    // Check to see that the grid cells got the new value we set above.
    myArray.forEachCell(box, [&](const AABB &cell, Morton3 index, const int &value){
        XCTAssertEqual(42, myArray.get(cell.center));
        XCTAssertEqual(42, myArray.get(index));
        XCTAssertEqual(42, value);
    });
    
    // Throws an exception when the region is not in-bounds.
    try {
        const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
        myArray.mutableForEachCell(negCell, [&](const AABB &cell, Morton3 index, int &value){
            SDL_Log("{(%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)}",
                    cell.center.x, cell.center.y, cell.center.z,
                    cell.extent.x, cell.extent.y, cell.extent.z);
            return 42;
        });
        XCTFail("We expected an exception before reaching this line.");
    } catch(const OutOfBoundsException &e) {
        // Swallow the exception without failing the test.
    }
    
    // If the region is a zero-sized box then we should iterate over no cells.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    actualCells.clear();
    myArray.mutableForEachCell(zeroBox, [&](const AABB &cell, Morton3 index, int &value){
        XCTFail("We expected to iterate over zero cells in this case.");
        value = 42;
    });
}

- (void) testIndexing {
    AABB box = {vec3(64.5f, 128.5f, 64.5f), vec3(80.f, 144.f, 80.f)};
    ivec3 res(160, 288, 160);
    Array3D<int> myArray(box, res);
    XCTAssertEqual(myArray.gridResolution(), res);
    XCTAssertEqual(myArray.boundingBox(), box);
    XCTAssertEqual(myArray.cellDimensions(), vec3(1.0, 1.0f, 1.0f));
    
    Morton3 index;
    
    for (size_t x = 0; x < res.x; ++x) {
        for (size_t y = 0; y < res.y; ++y) {
            for (size_t z = 0; z < res.z; ++z) {
                index = myArray.indexAtCellCoords(ivec3(x, y, z));
                XCTAssertTrue(myArray.isValidIndex(index));
            }
        }
    }
    
    XCTAssertFalse(myArray.isValidIndex(myArray.indexAtCellCoords(res)));
    
    vec3 point(-15.f, 241.f, -15.f);
    index = myArray.indexAtPoint(point);
    XCTAssertTrue(myArray.isValidIndex(index));
    myArray.mutableReference(index) = 42;
    XCTAssertEqual(myArray.get(index), 42);
    XCTAssertEqual(myArray.get(point), 42);
}

@end
