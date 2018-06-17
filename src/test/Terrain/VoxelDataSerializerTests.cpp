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
#include "Terrain/VoxelDataChunk.hpp"

TEST_CASE("Test Voxel Serializer Round Trip for Array Chunks", "[VoxelDataSerializer]") {
    VoxelDataGenerator generator(0);
    VoxelDataSerializer serializer;
    const AABB region{{16, 16, 16},{16, 16, 16}};
    const VoxelDataChunk originalChunk = VoxelDataChunk::createArrayChunk(generator.copy(region));
    const auto serializedBytes = serializer.store(originalChunk);
    const VoxelDataChunk reconstructedChunk = serializer.load(region, serializedBytes);
    REQUIRE(originalChunk.getUncompressedBytes() == reconstructedChunk.getUncompressedBytes());
}

TEST_CASE("Test Voxel Serializer Round Trip for Sky Chunks", "[VoxelDataSerializer]") {
    VoxelDataSerializer serializer;
    const AABB region{{16, 16, 16},{16, 16, 16}};
    const glm::ivec3 gridResolution(32);
    const VoxelDataChunk originalChunk = VoxelDataChunk::createSkyChunk(region, gridResolution);
    const auto serializedBytes = serializer.store(originalChunk);
    const VoxelDataChunk reconstructedChunk = serializer.load(region, serializedBytes);
    REQUIRE(originalChunk.getUncompressedBytes() == reconstructedChunk.getUncompressedBytes());
}

TEST_CASE("Test Voxel Serializer Round Trip for Ground Chunks", "[VoxelDataSerializer]") {
    VoxelDataSerializer serializer;
    const AABB region{{16, 16, 16},{16, 16, 16}};
    const glm::ivec3 gridResolution(32);
    const VoxelDataChunk originalChunk = VoxelDataChunk::createGroundChunk(region, gridResolution);
    const auto serializedBytes = serializer.store(originalChunk);
    const VoxelDataChunk reconstructedChunk = serializer.load(region, serializedBytes);
    REQUIRE(originalChunk.getUncompressedBytes() == reconstructedChunk.getUncompressedBytes());
}
