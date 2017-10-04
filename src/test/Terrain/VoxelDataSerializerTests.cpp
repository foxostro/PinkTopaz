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
    
    const auto original = generator.copy(region);
    const auto bytes = serializer.store(original);
    const auto reconstructed = serializer.load(region, bytes);
    REQUIRE(original == reconstructed);
}
