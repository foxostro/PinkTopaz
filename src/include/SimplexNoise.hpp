//
//  SimplexNoise.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/16/17.
//

// Simplex noise functions based on paper and example code from <http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf>.

#ifndef SimplexNoise_hpp
#define SimplexNoise_hpp

#include <glm/glm.hpp>

class SimplexNoise
{
public:
    ~SimplexNoise() = default;
    
    SimplexNoise(unsigned seed);
    
    float noiseAtPoint(const glm::vec3 &p) const;
    
    inline float noiseAtPointWithFourOctaves(const glm::vec3 &p) const
    {
        float noise;
        
        noise =  noiseAtPoint(p * 1.0f) * 0.5000f;
        noise += noiseAtPoint(p * 2.0f) * 0.2500f;
        noise += noiseAtPoint(p * 4.0f) * 0.1250f;
        noise += noiseAtPoint(p * 8.0f) * 0.0625f;
        
        return noise;
    }
    
private:
    // Skewing and unskewing factors for 3D.
    static constexpr float F3 = 1.0/3.0;
    static constexpr float G3 = 1.0/6.0;
    
    static const glm::vec3 grad3[];
    
    // To remove the need for index wrapping, double the permutation table length
    unsigned char perm[512];
    unsigned char permMod12[512];
};

#endif /* SimplexNoise_hpp */
