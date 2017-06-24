//
//  Noise.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/24/12.
//  Copyright 2012-2017 Andrew Fox. All rights reserved.
//

#ifndef Noise_hpp
#define Noise_hpp

#include <glm/glm.hpp>

struct NoiseContext;

class Noise
{
public:
    ~Noise();
    Noise(unsigned seed);
    float noiseAtPoint(const glm::vec3 &p) const;
    float noiseAtPointWithFourOctaves(const glm::vec3 &p) const;
    
private:
    NoiseContext *_context;
};

#endif /* Noise_hpp */
