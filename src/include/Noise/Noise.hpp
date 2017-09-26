//
//  Noise.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/16/17.
//

#ifndef Noise_hpp
#define Noise_hpp

#include <glm/glm.hpp>

// Interface for a smooth, spatial noise generator.
class Noise
{
public:
    // Generate noise at the specified point in space.
    virtual float noiseAtPoint(const glm::vec3 &p) const = 0;
    
    // Generate noise at the specified point in space, but apply four octaves
    // of detail to the result.
    inline float noiseAtPointWithFourOctaves(const glm::vec3 &p) const
    {
        float noise;
        
        noise =  noiseAtPoint(p * 1.0f) * 0.5000f;
        noise += noiseAtPoint(p * 2.0f) * 0.2500f;
        noise += noiseAtPoint(p * 4.0f) * 0.1250f;
        noise += noiseAtPoint(p * 8.0f) * 0.0625f;
        
        return noise;
    }
};

#endif /* Noise_hpp */
