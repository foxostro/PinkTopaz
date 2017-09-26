//
//  SimplexNoise.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/16/17.
//

#ifndef SimplexNoise_hpp
#define SimplexNoise_hpp

#include "Noise.hpp"

class SimplexNoise : public Noise
{
public:
    SimplexNoise() : SimplexNoise(0) {}
    SimplexNoise(unsigned seed = 0);
    float noiseAtPoint(const glm::vec3 &p) const override;
    
protected:
    // Skewing and unskewing factors for 3D.
    static constexpr float F3 = 1.0/3.0;
    static constexpr float G3 = 1.0/6.0;
    
    static const glm::vec3 grad3[];
    
    // To remove the need for index wrapping, double the permutation table length
    unsigned char perm[512];
    unsigned char permMod12[512];
};

#endif /* SimplexNoise_hpp */
