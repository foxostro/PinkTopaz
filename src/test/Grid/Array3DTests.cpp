//
//  Array3DTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/24/17.
//
//

#include <fmt/format.h>
#include "catch.hpp"
#include "Grid/Array3D.hpp"
#include "Grid/GridIndexerRange.hpp"
#include "Grid/GridPoints.hpp"

using glm::vec3;
using glm::ivec3;

TEST_CASE("Test Cell Coords At Point", "[Array3D]") {
	AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
	ivec3 res(4, 4, 4);
	Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    ivec3 a, b;
    
    // Adding the cell dimensions to the cell's minimum-corner takes us to the
    // neighboring cell. The range is not inclusive.
    a = myArray.cellCoordsAtPoint(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPoint(myArray.cellDimensions());
    REQUIRE((a + ivec3(1, 1, 1)) == b);
    
    // Two points which reside in the same cell result in the same cell coords.
    a = myArray.cellCoordsAtPoint(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPoint(myArray.cellDimensions() * 0.5f);
    REQUIRE(a == b);
}

TEST_CASE("Test Cell Coords At Point Round Up", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    ivec3 a, b;
    
    // Adding the cell dimensions to the cell's minimum-corner takes us to the
    // neighboring cell. The range is not inclusive.
    a = myArray.cellCoordsAtPointRoundUp(vec3(0.0f, 0.0f, 0.0f));
    b = myArray.cellCoordsAtPointRoundUp(myArray.cellDimensions());
    REQUIRE((a + ivec3(1, 1, 1)) == b);
    
    // We round up each coordinate so a cell at <0.5, 0.5, 0.5> will return the
    // coordinates for the cell at <1, 1, 1> and not <0, 0, 0> as we would see
    // with `cellCoordsAtPoint'.
    a = ivec3(1, 1, 1);
    b = myArray.cellCoordsAtPointRoundUp(myArray.cellDimensions() * 0.5f);
    REQUIRE(a == b);
}

TEST_CASE("Test Cell Center At Cell Coords", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    REQUIRE(myArray.cellCenterAtCellCoords(ivec3(0, 0, 0)) == vec3(0.5f, 0.5f, 0.5f));
    REQUIRE(myArray.cellCenterAtCellCoords(ivec3(2, 2, 2)) == vec3(2.5f, 2.5f, 2.5f));
    REQUIRE(myArray.cellCenterAtCellCoords(ivec3(3, 3, 3)) == vec3(3.5f, 3.5f, 3.5f));
}

TEST_CASE("Test Cell Center At Point", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    // Test a point on the minimum edge of a cell.
    REQUIRE(myArray.cellCenterAtPoint(vec3(0.0f, 0.0f, 0.0f)) == vec3(0.5f, 0.5f, 0.5f));
    
    // Test some arbitrary point within the cell.
    REQUIRE(myArray.cellCenterAtPoint(vec3(0.7f, 0.7f, 0.7f)) == vec3(0.5f, 0.5f, 0.5f));
    
    // Test a point on the maximum edge of the cell. This range of the cell is
    // exclusive so this edge is actually in a different cell.
    REQUIRE(myArray.cellCenterAtPoint(vec3(1.0f, 1.0f, 1.0f)) != vec3(0.5f, 0.5f, 0.5f));
}

TEST_CASE("Test Cell At Point", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    AABB cell = myArray.cellAtPoint(vec3(0.1f, 0.1f, 0.1f));
    REQUIRE(cell.center == vec3(0.5f, 0.5f, 0.5f));
    REQUIRE(cell.extent == vec3(0.5f, 0.5f, 0.5f));
    
    cell = myArray.cellAtPoint(vec3(1.0f, 0.5f, 0.5f));
    REQUIRE(cell.center == vec3(1.5f, 0.5f, 0.5f));
    REQUIRE(cell.extent == vec3(0.5f, 0.5f, 0.5f));
    
    cell = myArray.cellAtPoint(vec3(1.0f, 1.0f, 1.0f));
    REQUIRE(cell.center == vec3(1.5f, 1.5f, 1.5f));
    REQUIRE(cell.extent == vec3(0.5f, 0.5f, 0.5f));
}

TEST_CASE("Test Count Cells In Region", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    // If we pass in the exact bounding box then we should get the original grid
    // resolution back again.
    REQUIRE(res == myArray.countCellsInRegion(box));
    
    ivec3 c;
    
    // A region covering exactly one cell.
    const AABB oneCell = {vec3(0.5f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f)};
    c = myArray.countCellsInRegion(oneCell);
    REQUIRE(ivec3(1, 1, 1) == c);
    
    // The maximum-corner of a region is exclusive and is not part of the region
    // which means that a zero-sized region contains zero cells.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    c = myArray.countCellsInRegion(zeroBox);
    REQUIRE(ivec3(0, 0, 0) == c);
    
    // Currently, countCellsInRegion() returns <0,0,0> when the min and max
    // corners of the box are in the same cell. I'm not sure this is correct
    // behavior, but it is the current behavior and the test verifies that this
    // behavior does not change unintentionally.
    const AABB tenthBox1 = {vec3(0.1f, 0.1f, 0.1f), vec3(0.1f, 0.1f, 0.1f)};
    c = myArray.countCellsInRegion(tenthBox1);
    REQUIRE(ivec3(0, 0, 0) == c);
    
    // Currently, countCellsInRegion() returns <1,1,1> when the min and max
    // corners of the box are in adjacent cells. I'm not sure this is correct
    // behavior, but it is the current behavior and the test verifies that this
    // behavior does not change unintentionally.
    const AABB tenthBox2 = {vec3(1.0f, 1.0f, 1.0f), vec3(0.1f, 0.1f, 0.1f)};
    c = myArray.countCellsInRegion(tenthBox2);
    REQUIRE(ivec3(1, 1, 1) == c);
}

TEST_CASE("Test Snap Region To Cell Boundaries", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    AABB a, b;
    
    // If we pass in the exact bounding box then we should get the original box
    // since it's already snapped to cell boundaries.
    a = box;
    b = myArray.snapRegionToCellBoundaries(box);
    REQUIRE(a == b);
    
    // A zero size box is snapped to the center of the cell that its center
    // falls within.
    const AABB zeroBox1 = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    a = {vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 0.0f, 0.0f)};
    b = myArray.snapRegionToCellBoundaries(zeroBox1);
    REQUIRE(a == b);
    
    // Try a box that includes only one cell.
    const AABB oneCellBox = {vec3(0.5, 0.5, 0.5f), vec3(0.1f, 0.1f, 0.1f)};
    a = myArray.cellAtPoint(oneCellBox.center);
    b = myArray.snapRegionToCellBoundaries(oneCellBox);
    REQUIRE(a == b);
    
    // Try a box that covers several cells and which does not already cover
    // exact cell dimensions.
    const AABB box2 = {vec3(0.9f, 0.9f, 0.9f), vec3(1.0f, 1.0f, 1.0f)};
    a = myArray.snapRegionToCellBoundaries(box2);
    b = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    REQUIRE(a == b);
}

TEST_CASE("Test In Bounds", "[Array3D]") {
    AABB box = {vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f)};
    ivec3 res(4, 4, 4);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    // The grid bounding box is obviously within the valid space of the grid.
    REQUIRE(myArray.inbounds(box));
    
    // A zero-sized region within the valid space of the grid is in-bounds much
    // the same as a point would be.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    REQUIRE(myArray.inbounds(zeroBox));
    
    // A small box within the grid is in-bounds.
    const AABB tenthBox = {vec3(0.1f, 0.1f, 0.1f), vec3(0.1f, 0.1f, 0.1f)};
    REQUIRE(myArray.inbounds(tenthBox));
    
    // A region totally outside the grid bounding box is not in-bounds.
    const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
    REQUIRE_FALSE(myArray.inbounds(negCell));
    
    // A region which intersects the bounding box but is not wholly within the
    // box is not considered in-bounds.
    const AABB pastMin = {vec3(0.5f, 0.5f, 0.5f), vec3(0.7f, 0.7f, 0.7f)};
    REQUIRE_FALSE(myArray.inbounds(pastMin));
}

TEST_CASE("Test Iteration Over Cells", "[Array3D]") {
    AABB box = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    ivec3 res(2, 2, 2);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
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
    
    for (const auto &cellCoords : slice(myArray, box)) {
        const auto cell = myArray.cellAtCellCoords(cellCoords);
        actualCells.push_back(cell);
    }
    
    // Make sure we got all the cells we intended to.
    REQUIRE(desiredCells == actualCells);
    
    // Throws an exception when the region is not in-bounds.
    const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
    REQUIRE_THROWS_AS(slice(myArray, negCell), OutOfBoundsException);
    
    // If the region is a zero-sized box then we should iterate over no cells.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    actualCells.clear();
    for (const auto &cellCoords : slice(myArray, zeroBox)) {
        REQUIRE(!"We expected to iterate over zero cells in this case.");
    }
    
    // Make sure we can mutate cells and get back the changed values.
    for (const auto &cellCoords : slice(myArray, box)) {
        myArray.mutableReference(cellCoords) = 42;
    }
    for (const auto &cellCoords : slice(myArray, box)) {
        REQUIRE(42 == myArray.reference(cellCoords));
    }
}

TEST_CASE("Test For Points In Grid", "[Array3D]") {
    AABB box = {vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)};
    ivec3 res(2, 2, 2);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
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
    
    for (const vec3 point : points(myArray, box)) {
        actualPoints.push_back(point);
    }
    
    // Make sure we got all the points we intended to.
    REQUIRE(desiredPoints == actualPoints);
    
    // Throws an exception when the region is not in-bounds.
    const AABB negCell = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f)};
    
    REQUIRE_THROWS_AS(points(myArray, negCell), OutOfBoundsException);
    
    // If the region is a zero-sized box then we should iterate over one point.
    const AABB zeroBox = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)};
    actualPoints.clear();
    for (const vec3 point : points(myArray, zeroBox)) {
        actualPoints.push_back(point);
    }
    REQUIRE(1 == actualPoints.size());
}

TEST_CASE("Test Indexing", "[Array3D]") {
    AABB box = {vec3(64.5f, 128.5f, 64.5f), vec3(80.f, 144.f, 80.f)};
    ivec3 res(160, 288, 160);
    Array3D<int> myArray(box, res);
    REQUIRE(myArray.gridResolution() == res);
    REQUIRE(myArray.boundingBox() == box);
    REQUIRE(myArray.cellDimensions() == vec3(1.0, 1.0f, 1.0f));
    
    Morton3 index;
    
    for (int x = 0; x < res.x; ++x) {
        for (int y = 0; y < res.y; ++y) {
            for (int z = 0; z < res.z; ++z) {
                index = myArray.indexAtCellCoords(ivec3(x, y, z));
                REQUIRE(myArray.isValidIndex(index));
            }
        }
    }
    
    REQUIRE_FALSE(myArray.isValidIndex(myArray.indexAtCellCoords(res)));
    
    vec3 point(-15.f, 241.f, -15.f);
    index = myArray.indexAtPoint(point);
    REQUIRE(myArray.isValidIndex(index));
    myArray.mutableReference(index) = 42;
    REQUIRE(myArray.reference(index) == 42);
    REQUIRE(myArray.reference(point) == 42);
}
