//
//  VoxelDataSerializerTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/3/17.
//
//

#include "catch.hpp"
#include "Terrain/VoxelDataSerializer.hpp"
#include "Terrain/VoxelDataGenerator.hpp"

TEST_CASE("Test Voxel Serializer Round Trip", "[VoxelDataSerializer]") {
    VoxelDataGenerator generator(0);
    VoxelDataSerializer serializer;
    const AABB region{{16, 16, 16},{16, 16, 16}};
    
    generator.readerTransaction(region, [&](const Array3D<Voxel> &original){
        const auto bytes = serializer.store(original);
        const auto reconstructed = serializer.load(region, bytes);
        REQUIRE(original == reconstructed);
    });
}

TEST_CASE("Test Voxel Serializer Pad The End", "[VoxelDataSerializer]") {
    VoxelDataGenerator generator(0);
    VoxelDataSerializer serializer;
    const AABB region{{16, 16, 16},{16, 16, 16}};
    
    generator.readerTransaction(region, [&](const Array3D<Voxel> &original){
        auto bytes = serializer.store(original);
        
        // Make sure we can reconstruct the chunk even after padding the end of
        // the serialized data.
        for (size_t i = 0; i < 100; ++i) {
            bytes.push_back(rand() % 255);
        }
        
        const auto reconstructed = serializer.load(region, bytes);
        REQUIRE(original == reconstructed);
    });
}
