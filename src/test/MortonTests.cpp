//
//  MortonTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/24/17.
//
//

#include "catch.hpp"
#include <glm/glm.hpp>
#include "Morton.hpp"

using namespace glm;

TEST_CASE("Test Zero", "[Morton]") {
    const ivec3 zero(0, 0, 0);
    const Morton3 morton(zero);
    const size_t code = (size_t)morton;
    REQUIRE(code == 0);
}

TEST_CASE("Test Encode 0xF", "[Morton]") {
    // Interleaving bits of the morton code gives us back the coordinates.
    const ivec3 point(0b1111, 0b1111, 0b1111);
    const Morton3 morton(point);
    const size_t code = (size_t)morton;
    REQUIRE(code == 0b111111111111);
}

TEST_CASE("Test Encode 0xA", "[Morton]") {
    const ivec3 point(0b1110, 0b1110, 0b1110);
    const Morton3 morton(point);
    const size_t code = (size_t)morton;
    REQUIRE(code == 0b111111111000);
}

TEST_CASE("Test Decode", "[Morton]") {
    // De-interleaving bits of the morton code gives us back the coordinates.
    REQUIRE(ivec3(0b1110, 0b1110, 0b1110) == Morton3::decode(0b111111111000));
    REQUIRE(ivec3(0b1010, 0b1111, 0b1111) == Morton3::decode(0b111110111110));
}

TEST_CASE("Test Increment and Decrement", "[Morton]") {
    const Morton3 a(ivec3(10, 11, 12));
    Morton3 b(a);
    
    b.incX();
    REQUIRE(b.decode() == ivec3(11, 11, 12));
    
    b.incY();
    REQUIRE(b.decode() == ivec3(11, 12, 12));
    
    b.incZ();
    REQUIRE(b.decode() == ivec3(11, 12, 13));
    
    b.decX();
    REQUIRE(b.decode() == ivec3(10, 12, 13));
    
    b.decY();
    REQUIRE(b.decode() == ivec3(10, 11, 13));
    
    b.decZ();
    REQUIRE(b.decode() == ivec3(10, 11, 12));
}
