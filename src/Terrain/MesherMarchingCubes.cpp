//
//  MesherMarchingCubes.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/16.
//
//

#include "Terrain/MesherMarchingCubes.hpp"
#include "Renderer/TerrainVertex.hpp"
#include "math.hpp" // for clamp
#include <glm/glm.hpp>
#include <array>

using glm::vec3;
using glm::vec4;

static constexpr float L = 0.5f;
static const glm::vec3 LLL(L, L, L);
static const glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
static const glm::vec3 texCoord(0.0f, 0.0f, 0.0f);

void MesherMarchingCubes::polygonizeGridCell(StaticMesh &geometry,
                                             const std::array<CubeVertex, NUM_CUBE_VERTS> &cube,
                                             float isosurface)
{
    // Based on Paul Bourke's Marching Cubes algorithm at
    // <http://paulbourke.net/geometry/polygonise/>. The edge and tri tables
    // come directly from the sample code in the article.
    
    constexpr unsigned edgeTable[256] = {
#include "edgetable.def"
    };
    
    constexpr int triTable[256][16] = {
#include "tritable.def"
    };
    
    // Build an index to look into the tables. Examine each of the eight
    // neighboring cells and set a bit in the index to '0' or '1' depending
    // on whether the neighboring voxel is empty or not-empty.
    unsigned index = 0;
    for(size_t i = 0; i < NUM_CUBE_VERTS; ++i)
    {
        if (cube[i].voxel.value > isosurface) {
            index |= (1 << i);
        }
    }
    
    // If all neighbors are empty then there's nothing to do. Bail out early.
    if (edgeTable[index] == 0) {
        return;
    }
    
    // For each intersection between the surface and the cube, record the
    // indices of the two cube vertices on either side of the intersection.
    // We interpolate the vertices later, when emitting triangles.
    std::array<std::pair<size_t, size_t>, NUM_CUBE_EDGES> vertexList;
    
    {
        constexpr size_t intersect1[NUM_CUBE_EDGES] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3};
        constexpr size_t intersect2[NUM_CUBE_EDGES] = {1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7};
        
        for(size_t i = 0; i < NUM_CUBE_EDGES; ++i)
        {
            if (edgeTable[index] & (1 << i)) {
                vertexList[i] = std::make_pair(intersect1[i], intersect2[i]);
            }
        }
    }
    
    for(size_t i = 0; triTable[index][i] != -1; i += 3)
    {
        const std::array<std::pair<size_t, size_t>, 3> pairs = {{
            vertexList[triTable[index][i+2]],
            vertexList[triTable[index][i+1]],
            vertexList[triTable[index][i+0]]
        }};
        
        for (size_t i = 0; i < 3; ++i)
        {
            const vec3 &v1 = cube[pairs[i].first].worldPos;
            const vec3 &v2 = cube[pairs[i].second].worldPos;
            vec3 worldPos = glm::mix(v1, v2, LLL);
            
            geometry.addVertex(TerrainVertex(vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f),
                                             color,
                                             texCoord));
        }
    }
}

StaticMesh MesherMarchingCubes::extract(const VoxelData &voxels,
                                        const AABB &aabb,
                                        float isosurface)
{
    StaticMesh geometry;
    
    // Offset to align with the grid cells used by marching cubes.
    const AABB insetAABB = {aabb.center, aabb.extent - LLL};
    
    // Marching Cubes isosurface extraction. This is embarrassingly parallel
    // and could potentially benefit from being split across multiple
    // threads, say, with OpenMP or similar.
    voxels.forPointsInGrid(insetAABB, [&](const glm::vec3 &pos){
        const std::array<CubeVertex, NUM_CUBE_VERTS> cube = {{
            CubeVertex(voxels, pos + vec3(-L, -L, +L)),
            CubeVertex(voxels, pos + vec3(+L, -L, +L)),
            CubeVertex(voxels, pos + vec3(+L, -L, -L)),
            CubeVertex(voxels, pos + vec3(-L, -L, -L)),
            CubeVertex(voxels, pos + vec3(-L, +L, +L)),
            CubeVertex(voxels, pos + vec3(+L, +L, +L)),
            CubeVertex(voxels, pos + vec3(+L, +L, -L)),
            CubeVertex(voxels, pos + vec3(-L, +L, -L))
        }};
        
        polygonizeGridCell(geometry, cube, isosurface);
    });
    
    return geometry;
}
