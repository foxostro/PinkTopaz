#import <XCTest/XCTest.h>
#import "Renderer/StaticMeshSerializer.hpp"
#import "FileUtilities.hpp"

@interface StaticMeshSerializerTests : XCTestCase

@end

@implementation StaticMeshSerializerTests

- (void)testRoundTrip {
    // Load a mesh and make sure we produce the same bytes when saving it again.
    StaticMeshSerializer serializer;
    std::vector<uint8_t> expectedBytes = binaryFileContents("test_data/terrain.3d.bin");
    StaticMesh mesh = serializer.load(expectedBytes);
    std::vector<uint8_t> actualBytes = serializer.save(mesh);
    XCTAssertEqual(expectedBytes, actualBytes);
}

@end
