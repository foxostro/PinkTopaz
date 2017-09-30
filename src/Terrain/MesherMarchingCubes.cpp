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
#include "SDL.h"

using glm::vec3;
using glm::vec4;

static constexpr float L = 0.5f;
static const vec3 LLL(L, L, L);
static const vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

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
        
        // Calculate one normal for the entire face.
        // AFOX_TODO: This can probably be cleaned up a bit. We're mixing `cellRelativeVertexPos' both here and below.
        vec3 n;
        {
            CubeVertex v1[3] = {
                cube[pairs[0].first],
                cube[pairs[1].first],
                cube[pairs[2].first]
            };
            
            CubeVertex v2[3] = {
                cube[pairs[0].second],
                cube[pairs[1].second],
                cube[pairs[2].second]
            };
            
            vec3 cellRelativeVertexPos[3] = {
                glm::mix(v1[0].cellRelativeVertexPos, v2[0].cellRelativeVertexPos, LLL),
                glm::mix(v1[1].cellRelativeVertexPos, v2[1].cellRelativeVertexPos, LLL),
                glm::mix(v1[2].cellRelativeVertexPos, v2[2].cellRelativeVertexPos, LLL)
            };
            
            n = glm::normalize(glm::cross(cellRelativeVertexPos[1] - cellRelativeVertexPos[0],
                                          cellRelativeVertexPos[2] - cellRelativeVertexPos[0]));
        }
        
        for (size_t i = 0; i < 3; ++i)
        {
            const CubeVertex &v1 = cube[pairs[i].first];
            const CubeVertex &v2 = cube[pairs[i].second];
            vec3 worldPos = glm::mix(v1.worldPos, v2.worldPos, LLL);
            vec3 cellRelativeVertexPos = glm::mix(v1.cellRelativeVertexPos, v2.cellRelativeVertexPos, LLL);
            
            // Compute texture coordinates for the vertex. We want textures to
            // tile every cell. We need to consider the face normal to make sure
            // that vertical faces are textured correctly too.
            vec3 texCoord;
            {
                if (n.y == 0) {
                    if (n.x != 0) {
                        texCoord = vec3(cellRelativeVertexPos.z, cellRelativeVertexPos.y, 0.0f);
                    } else {
                        texCoord = vec3(cellRelativeVertexPos.x, cellRelativeVertexPos.y, 0.0f);
                    }
                } else if (n.y > 0) {
                    texCoord = vec3(cellRelativeVertexPos.x, cellRelativeVertexPos.z, 0.0f);
                } else {
                    texCoord = vec3(cellRelativeVertexPos.x, cellRelativeVertexPos.z, 0.0f);
                }
                
                // The Z-coordinate is an index into the texture array.
                constexpr float grassTex = 75.f;
                texCoord.z = grassTex;
            }
            
            geometry.addVertex(TerrainVertex(vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f),
                                             color,
                                             texCoord));
        }
    }
}

StaticMesh MesherMarchingCubes::extract(const Array3D<Voxel> &voxels,
                                        const AABB &aabb,
                                        float isosurface)
{
    StaticMesh geometry;
    
    // Offset to align with the grid cells used by marching cubes.
    const AABB insetAABB = aabb.inset(LLL);
    
    static const vec3 posOffset[NUM_CUBE_VERTS] = {
        vec3(-L, -L, +L),
        vec3(+L, -L, +L),
        vec3(+L, -L, -L),
        vec3(-L, -L, -L),
        vec3(-L, +L, +L),
        vec3(+L, +L, +L),
        vec3(+L, +L, -L),
        vec3(-L, +L, -L)
    };
    
    for (const vec3 pos : voxels.points(insetAABB)) {
        const vec3 vertexPositions[NUM_CUBE_VERTS] = {
            pos + posOffset[0],
            pos + posOffset[1],
            pos + posOffset[2],
            pos + posOffset[3],
            pos + posOffset[4],
            pos + posOffset[5],
            pos + posOffset[6],
            pos + posOffset[7]
        };
        
        const std::array<CubeVertex, NUM_CUBE_VERTS> cube = {{
            CubeVertex(voxels.reference(vertexPositions[0]), vertexPositions[0], vec3(0.0f, 0.0f, 1.0f)),
            CubeVertex(voxels.reference(vertexPositions[1]), vertexPositions[1], vec3(1.0f, 0.0f, 1.0f)),
            CubeVertex(voxels.reference(vertexPositions[2]), vertexPositions[2], vec3(1.0f, 0.0f, 0.0f)),
            CubeVertex(voxels.reference(vertexPositions[3]), vertexPositions[3], vec3(0.0f, 0.0f, 0.0f)),
            CubeVertex(voxels.reference(vertexPositions[4]), vertexPositions[4], vec3(0.0f, 1.0f, 1.0f)),
            CubeVertex(voxels.reference(vertexPositions[5]), vertexPositions[5], vec3(1.0f, 1.0f, 1.0f)),
            CubeVertex(voxels.reference(vertexPositions[6]), vertexPositions[6], vec3(1.0f, 1.0f, 0.0f)),
            CubeVertex(voxels.reference(vertexPositions[7]), vertexPositions[7], vec3(0.0f, 1.0f, 0.0f)),
        }};
        
        polygonizeGridCell(geometry, cube, isosurface);
    }
    
    return geometry;
}
