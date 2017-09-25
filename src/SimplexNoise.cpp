// Simplex noise functions based on paper and example code from <http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf>.
/*
 * A speed-improved simplex noise algorithm for 2D, 3D and 4D in Java.
 *
 * Based on example code by Stefan Gustavson (stegu@itn.liu.se).
 * Optimisations by Peter Eastman (peastman@drizzle.stanford.edu).
 * Better rank ordering method for 4D by Stefan Gustavson in 2012.
 *
 * This could be speeded up even further, but it's useful as it is.
 *
 * Version 2012-03-09
 *
 * This code was placed in the public domain by its original author,
 * Stefan Gustavson. You may use it as you see fit, but
 * attribution is appreciated.
 *
 */

#include "SimplexNoise.hpp"
#include <glm/gtx/component_wise.hpp>
#include <random>

using namespace glm;

const glm::vec3 SimplexNoise::grad3[] =
{
    glm::vec3(1,1,0), glm::vec3(-1,1,0), glm::vec3(1,-1,0), glm::vec3(-1,-1,0),
    glm::vec3(1,0,1), glm::vec3(-1,0,1), glm::vec3(1,0,-1), glm::vec3(-1,0,-1),
    glm::vec3(0,1,1), glm::vec3(0,-1,1), glm::vec3(0,1,-1), glm::vec3(0,-1,-1)
};

template<typename ElementType, typename RandomGenerator>
static void shuffle(ElementType *array, size_t count, RandomGenerator &&gen)
{
    assert(array);
    
    if (count <= 1) {
        return;
    }
    
    for (size_t i = 0; i < count - 1; i++) {
        size_t j = i + gen() / (gen.max() / (count - i) + 1);
        ElementType t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

SimplexNoise::SimplexNoise(unsigned seed)
{
    constexpr size_t N = 256;
    unsigned char permfill[N];
    
    for (int i = 0; i < N; ++i) {
        permfill[i] = i+1;
    }
    
    std::mt19937 e1(seed);
    shuffle(permfill, N, e1);
    
    for (int i = 0; i < N*2; ++i) {
        perm[i] = permfill[i & 255];
        permMod12[i] = perm[i] % 12;
    }
}

float SimplexNoise::noiseAtPoint(const glm::vec3 &p) const
{
    // Corners of the cell in (i,j,k) coords. The second and third corner coords
    // are given as offsets relative to the first.
    mat3x3 ijk;
    
    // Corners of the simplex cell. Given as distances from the cell origin.
    mat4x3 corner;
    
    // Skew the input space to determine which simplex cell we're in
    ijk[0] = vec3(floor(p + compAdd(p) * F3));
    
    // Unskew the cell origin back to (x,y,z) space
    corner[0] = p - vec3(ijk[0] - compAdd(ijk[0]) * G3);
    
    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    if (corner[0].x >= corner[0].y) {
        if (corner[0].y >= corner[0].z) {
            // X Y Z order
            ijk[1] = vec3(1.f, 0.f, 0.f);
            ijk[2] = vec3(1.f, 1.f, 0.f);
        } else if(corner[0].x >= corner[0].z) {
            // X Z Y order
            ijk[1] = vec3(1.f, 0.f, 0.f);
            ijk[2] = vec3(1.f, 0.f, 1.f);
        } else {
            // Z X Y order
            ijk[1] = vec3(0.f, 0.f, 1.f);
            ijk[2] = vec3(1.f, 0.f, 1.f);
        }
    } else { // x0<y0
        if (corner[0].y < corner[0].z) {
            // Z Y X order
            ijk[1] = vec3(0.f, 0.f, 1.f);
            ijk[2] = vec3(0.f, 1.f, 1.f);
        } else if (corner[0].x < corner[0].z) {
            // Y Z X order
            ijk[1] = vec3(0.f, 1.f, 0.f);
            ijk[2] = vec3(0.f, 1.f, 1.f);
        } else {
            // Y X Z order
            ijk[1] = vec3(0.f, 1.f, 0.f);
            ijk[2] = vec3(1.f, 1.f, 0.f);
        }
    }
    
    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    corner[1] = corner[0] - ijk[1] + G3;     // Offsets for second corner in (x,y,z) coords
    corner[2] = corner[0] - ijk[2] + 2.f*G3; // Offsets for third corner in (x,y,z) coords
    corner[3] = corner[0] - 1.f + 3.f*G3;    // Offsets for last corner in (x,y,z) coords
    
    // Calculate the gradient indices for the simplex cell.
    // This basically hashes the cell indices using the permutation tables that
    // got shuffled in the constructor.
    const int ii = (int)ijk[0].x & 255;
    const int jj = (int)ijk[0].y & 255;
    const int kk = (int)ijk[0].z & 255;
    const ivec4 gi(permMod12[ii + perm[jj + perm[kk]]],
                   permMod12[ii + (int)ijk[1].x + perm[jj + (int)ijk[1].y + perm[kk + (int)ijk[1].z]]],
                   permMod12[ii + (int)ijk[2].x + perm[jj + (int)ijk[2].y + perm[kk + (int)ijk[2].z]]],
                   permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]]);
    
    // Calculate the contribution from the four corners.
    
    // The interpolant function is f(t) = 6t^5 – 15t^4 + 10t^3
    // Various subexpressions of the interpolant have been extracted out and
    // are calculated for all corners simultaneously.
    
    const vec3 negCornerSqr[4] = {
        -vec3(corner[0].x*corner[0].x, corner[0].y*corner[0].y, corner[0].z*corner[0].z),
        -vec3(corner[1].x*corner[1].x, corner[1].y*corner[1].y, corner[1].z*corner[1].z),
        -vec3(corner[2].x*corner[2].x, corner[2].y*corner[2].y, corner[2].z*corner[2].z),
        -vec3(corner[3].x*corner[3].x, corner[3].y*corner[3].y, corner[3].z*corner[3].z),
    };
    
    const vec4 dotProducts(dot(grad3[gi.x], corner[0]),
                           dot(grad3[gi.y], corner[1]),
                           dot(grad3[gi.z], corner[2]),
                           dot(grad3[gi.w], corner[3]));
    
    const vec4 sums(compAdd(negCornerSqr[0]),
                    compAdd(negCornerSqr[1]),
                    compAdd(negCornerSqr[2]),
                    compAdd(negCornerSqr[3]));
    
    const vec4 t = 0.6f + sums;
    
    // If t[a]<0 then contributions[a] must be zero. We use `mask' to zero those
    // corners' contributions out.
    const vec4 mask((t.x < 0) ? 0 : 1, (t.y < 0) ? 0 : 1,
                    (t.z < 0) ? 0 : 1, (t.w < 0) ? 0 : 1);
    const vec4 contributions = t * t * t * t * dotProducts * mask;
    
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.f * compAdd(contributions);
}
