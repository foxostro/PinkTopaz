//
//  SimplexNoiseTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/25/17.
//
//

#include "catch.hpp"
#include "Noise/SimplexNoise.hpp"
#include <glm/glm.hpp>
#include <cstdlib>
#include <random>

using namespace glm;

template<typename RandomGenerator> inline
vec3 randomPoint(const vec3 &min, const vec3 &max, RandomGenerator &generator)
{
    std::uniform_real_distribution<decltype(min.x)> distx(min.x, max.x);
    std::uniform_real_distribution<decltype(min.y)> disty(min.y, max.y);
    std::uniform_real_distribution<decltype(min.z)> distz(min.z, max.z);
    return vec3(distx(generator), disty(generator), distz(generator));
}

TEST_CASE("SimplexNoise.noiseAtPoint() returns values in [-1, +1]", "[Noise]") {
    // Test that the noise generated for a large, random sample of points in
    // space is always in [-1, +1].

    constexpr size_t numSamplePoints = 1000;
    const vec3 min(-1000.f, -1000.f, -1000.f);
    const vec3 max(+1000.f, +1000.f, +1000.f);
    std::default_random_engine generator; // Use the default random seed.

    const auto noiseSeed = generator();
    SimplexNoise noise(noiseSeed);

    // Now test a bunch of random points in space.
    for (size_t i = 0; i < numSamplePoints; ++i) {
        const vec3 p = randomPoint(min, max, generator);
        const auto a = noise.noiseAtPoint(p);
        REQUIRE(a >= -1);
        REQUIRE(a <= +1);
    }
}
