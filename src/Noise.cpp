//
//  GSNoise.m
//  GutsyStorm
//
//  Created by Andrew Fox on 3/24/12.
//  Copyright © 2012-2016 Andrew Fox. All rights reserved.
//

#include "Noise.hpp"
#include "Exception.hpp"
#include "snoise3.h"

Noise::~Noise()
{
    FeepingCreature_DestroyNoiseContext(_context);
}

Noise::Noise(unsigned seed)
{
    _context = FeepingCreature_CreateNoiseContext(&seed);
    if (!_context) {
        throw Exception("Failed to create noise context.");
    }
}

float Noise::noiseAtPoint(const glm::vec3 &p) const
{
    return FeepingCreature_noise3(_context, p.x, p.y, p.z);
}

float Noise::noiseAtPointWithFourOctaves(const glm::vec3 &p) const
{
    float noise;
    
    noise =  noiseAtPoint(p * 1.0f) * 0.5000f;
    noise += noiseAtPoint(p * 2.0f) * 0.2500f;
    noise += noiseAtPoint(p * 4.0f) * 0.1250f;
    noise += noiseAtPoint(p * 8.0f) * 0.0625f;
    
    return noise;
}
