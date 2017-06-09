#import <XCTest/XCTest.h>
#import "Renderer/StaticMesh.hpp"
#import "Renderer/StaticMeshSerializer.hpp"
#import "Terrain/MesherNaiveSurfaceNets.hpp"
#import "Terrain/VoxelData.hpp"
#import "Terrain/VoxelDataLoader.hpp"
#import "FileUtilities.hpp"

@interface MesherNaiveSurfaceNetsTests : XCTestCase

@end

@implementation MesherNaiveSurfaceNetsTests

- (void)testCompareToGold {
    // Loads a block of voxels from file, extracts a mesh from those, and then
    // compares that mesh against a predetermined "golden" mesh. This ensures
    // that changes to the engine will not result in any unexpected changes to
    // the mesher output.
    StaticMeshSerializer meshSerializer;
    VoxelDataLoader voxelDataLoader;
    MesherNaiveSurfaceNets mesher;
    
    const auto voxelFileData = binaryFileContents("test_data/0_0_0.voxels.dat");
    const Array3D<Voxel> voxels = voxelDataLoader.createArray(voxelFileData, 1);
    StaticMesh actualMesh = mesher.extract(voxels, voxels.boundingBox().inset(glm::vec3(1.f, 1.f, 1.f)), 0.5f);
#if 0
    // Enable this block to generate a new golden mesh.
    const auto bytes = meshSerializer.save(actualMesh);
    saveBinaryFile("/Users/andrewfox/src/PinkTopaz/res/test_data/MesherNaiveSurfaceNetsTestsGold.bin", bytes);
#endif
    
    // Load the "golden" mesh.
    const auto goldMeshFileData = binaryFileContents("test_data/MesherNaiveSurfaceNetsTestsGold.bin");
    StaticMesh expectedMesh = meshSerializer.load(goldMeshFileData);
    
    XCTAssert(expectedMesh == actualMesh);
}

@end
