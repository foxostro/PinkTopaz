//
//  SimplexNoiseBenchmarks.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/25/17.
//
//

#include "Noise/SimplexNoise.hpp"

#include <glm/glm.hpp>
#include <cstdlib>
#include <random>
#include <chrono>
#include <iostream>

using namespace glm;

template<typename RandomGenerator> inline
vec3 randomPoint(const vec3 &min, const vec3 &max, RandomGenerator &generator)
{
    std::uniform_real_distribution<decltype(min.x)> distx(min.x, max.x);
    std::uniform_real_distribution<decltype(min.y)> disty(min.y, max.y);
    std::uniform_real_distribution<decltype(min.z)> distz(min.z, max.z);
    return vec3(distx(generator), disty(generator), distz(generator));
}

template<typename RandomGenerator>
static std::vector<vec3> generateSamplePoints(size_t numSamplePoints, RandomGenerator &generator)
{
    const vec3 min(-1000.f, -1000.f, -1000.f);
    const vec3 max(+1000.f, +1000.f, +1000.f);
    std::vector<vec3> samplePoints(numSamplePoints);
    for (size_t i = 0; i < numSamplePoints; ++i) {
        samplePoints.push_back(randomPoint(min, max, generator));
    }
    return samplePoints;
}

template<typename NoiseType>
static auto benchmarkNoise(NoiseType &noise, const std::vector<vec3> &samplePoints)
{
    const auto startTime = std::chrono::high_resolution_clock::now();
    for (const vec3 &p : samplePoints) {
        (void)noise.noiseAtPoint(p);
    }
    const auto finishTime = std::chrono::high_resolution_clock::now();
    return finishTime - startTime;
}

int main(int argc, char *argv[])
{
    std::default_random_engine generator;
    const auto seed = generator();
    const auto samplePoints = generateSamplePoints(1'000'000, generator);
    SimplexNoise simplexNoise(seed);

    const auto simplexNoiseDuration = benchmarkNoise(simplexNoise, samplePoints);
    
    using ms = std::chrono::milliseconds;
    
    std::cout << "SimplexNoise: "
              << std::chrono::duration_cast<ms>(simplexNoiseDuration).count()
              << " ms" << std::endl;

    return 0;
}
