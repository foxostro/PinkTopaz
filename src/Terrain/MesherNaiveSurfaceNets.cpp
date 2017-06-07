//
//  MesherNaiveSurfaceNets.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/5/17.
//
//

#include "Terrain/MesherNaiveSurfaceNets.hpp"
#include "Renderer/TerrainVertex.hpp"

using namespace glm;

static constexpr float L = 0.5f;
static const vec3 LLL(L, L, L);

std::array<glm::vec3, 4>
MesherNaiveSurfaceNets::quadForFace(const AABB &cell, size_t i)
{
    assert(i < NUM_FACES);
    
    static const std::array<std::array<vec3, 4>, NUM_FACES> quad = {{
        
        // FRONT
        {{
            vec3(-1.f,  1.f, 1.f),
            vec3( 1.f,  1.f, 1.f),
            vec3( 1.f, -1.f, 1.f),
            vec3(-1.f, -1.f, 1.f)
        }},
        
        // LEFT
        {{
            vec3(-1.f,  1.f,  1.f),
            vec3(-1.f, -1.f,  1.f),
            vec3(-1.f, -1.f, -1.f),
            vec3(-1.f,  1.f, -1.f)
        }},
        
        // BACK
        {{
            vec3( 1.f,  1.f, -1.f),
            vec3(-1.f,  1.f, -1.f),
            vec3(-1.f, -1.f, -1.f),
            vec3( 1.f, -1.f, -1.f)
        }},
        
        // RIGHT
        {{
            vec3(1.f,  1.f,  1.f),
            vec3(1.f,  1.f, -1.f),
            vec3(1.f, -1.f, -1.f),
            vec3(1.f, -1.f,  1.f)
        }},
        
        // TOP
        {{
            vec3( 1.f, 1.f,  1.f),
            vec3(-1.f, 1.f,  1.f),
            vec3(-1.f, 1.f, -1.f),
            vec3( 1.f, 1.f, -1.f)
        }},
        
        // BOTTOM
        {{
            vec3( 1.f, -1.f,  1.f),
            vec3( 1.f, -1.f, -1.f),
            vec3(-1.f, -1.f, -1.f),
            vec3(-1.f, -1.f,  1.f)
        }}
    }};
    
    const std::array<glm::vec3, 4> transformedQuad = {{
        cell.center + cell.extent * quad[i][0],
        cell.center + cell.extent * quad[i][1],
        cell.center + cell.extent * quad[i][2],
        cell.center + cell.extent * quad[i][3],
    }};
    
    return transformedQuad;
}

glm::vec3
MesherNaiveSurfaceNets::smoothVertex(const Array3D<Voxel> &voxels,
                                     float isosurface,
                                     const glm::vec3 &input)
{
    // AFOX_TODO: The cube sampling stuff in this method is similar to logic
    // found in the Marching Cubes implementation. The two can probably be
    // consolidated.
    
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
    
    const vec3 vertexPositions[NUM_CUBE_VERTS] = {
        input + posOffset[0],
        input + posOffset[1],
        input + posOffset[2],
        input + posOffset[3],
        input + posOffset[4],
        input + posOffset[5],
        input + posOffset[6],
        input + posOffset[7]
    };
    
    // AFOX_TODO: We can probably speed this part up a bit by calculating index
    // offsets ahead of time like we do for Marching Cubes.
    const std::array<CubeVertex, NUM_CUBE_VERTS> cube = {{
        CubeVertex(voxels.get(vertexPositions[0]), vertexPositions[0], vec3(0.0f, 0.0f, 1.0f)),
        CubeVertex(voxels.get(vertexPositions[1]), vertexPositions[1], vec3(1.0f, 0.0f, 1.0f)),
        CubeVertex(voxels.get(vertexPositions[2]), vertexPositions[2], vec3(1.0f, 0.0f, 0.0f)),
        CubeVertex(voxels.get(vertexPositions[3]), vertexPositions[3], vec3(0.0f, 0.0f, 0.0f)),
        CubeVertex(voxels.get(vertexPositions[4]), vertexPositions[4], vec3(0.0f, 1.0f, 1.0f)),
        CubeVertex(voxels.get(vertexPositions[5]), vertexPositions[5], vec3(1.0f, 1.0f, 1.0f)),
        CubeVertex(voxels.get(vertexPositions[6]), vertexPositions[6], vec3(1.0f, 1.0f, 0.0f)),
        CubeVertex(voxels.get(vertexPositions[7]), vertexPositions[7], vec3(0.0f, 1.0f, 0.0f)),
    }};
    
    constexpr unsigned edgeTable[256] = {
#include "edgetable.def"
    };
    
    // Build an index to look into the tables. Examine each of the eight
    // neighboring cells and set a bit in the index to '0' or '1' depending
    // on whether the neighboring voxel is empty or not-empty.
    unsigned index = 0;
    for (size_t i = 0; i < NUM_CUBE_VERTS; ++i) {
        if (cube[i].voxel.value > isosurface) {
            index |= (1 << i);
        }
    }
    
    // If we got into smoothVertex() at all then we expect an edge crossing.
    assert(edgeTable[index] != 0);
    
    // For each intersection between the surface and the cube, record the
    // indices of the two cube vertices on either side of the intersection.
    // We interpolate the vertices later, when emitting triangles.
    std::array<std::pair<size_t, size_t>, NUM_CUBE_EDGES> cubeEdges;
    
    {
        constexpr size_t intersect1[NUM_CUBE_EDGES] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3};
        constexpr size_t intersect2[NUM_CUBE_EDGES] = {1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7};
        
        for (size_t i = 0; i < NUM_CUBE_EDGES; ++i) {
            if (edgeTable[index] & (1 << i)) {
                cubeEdges[i] = std::make_pair(intersect1[i], intersect2[i]);
            } else {
                cubeEdges[i] = std::make_pair(NUM_CUBE_EDGES, NUM_CUBE_EDGES);
            }
        }
    }
    
    // Get the center of gravity of all edge crossings.
    glm::vec3 centerOfGravity;
    
    {
        float accumCount = 0.0f;
        vec3 accum(0.0f);
        
        for (const auto &pair : cubeEdges) {
            if (pair.first < NUM_CUBE_EDGES) {
                // NUM_CUBE_EDGES is expected to occur in pairs for the setinel value.
                assert(pair.second < NUM_CUBE_EDGES);
                
                // Interpolate vertex positions to get the edge crossing.
                const CubeVertex &v1 = cube[pair.first];
                const CubeVertex &v2 = cube[pair.second];
                const vec3 worldPos = glm::mix(v1.worldPos, v2.worldPos, LLL);
                
                accum += worldPos;
                accumCount++;
            }
        }
        
        centerOfGravity = accum * (1.0f / accumCount);
    }
    
    return centerOfGravity;
}

std::array<glm::vec3, 4>
MesherNaiveSurfaceNets::smoothQuad(const Array3D<Voxel> &voxels,
                                   float isosurface,
                                   const std::array<glm::vec3, 4> &input)
{
    std::array<glm::vec3, 4> output;
    
    for (size_t i = 0; i < 4; ++i) {
        output[i] = smoothVertex(voxels, isosurface, input[i]);
    }
    
    return output;
}

std::array<TerrainVertex, 6>
MesherNaiveSurfaceNets::verticesForFace(const Array3D<Voxel> &voxels,
                                        float isosurface,
                                        const AABB &cell,
                                        size_t face)
{
    static const vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    static const vec3 texCoord(0.0f, 0.0f, 0.0f);
    
    // Get the vertices for the specified face of the cell.
    const std::array<glm::vec3, 4> transformedQuad = quadForFace(cell, face);
    
    // Push the vertices toward the isosurface to smooth the surface.
    const std::array<glm::vec3, 4> smoothedQuad = smoothQuad(voxels, isosurface,
                                                             transformedQuad);
    
    // Stitch vertices of the quad together into two triangles.
    constexpr size_t n = 6;
    constexpr size_t indices[n] = { 0, 1, 2, 0, 2, 3 };
    const std::array<glm::vec3, n> vertexPositions = {{
        smoothedQuad[indices[0]],
        smoothedQuad[indices[1]],
        smoothedQuad[indices[2]],
        smoothedQuad[indices[3]],
        smoothedQuad[indices[4]],
        smoothedQuad[indices[5]]
    }};
    
    // Calculate normals for the two triangles.
    const vec3 n1 = normalize(cross(vertexPositions[1] - vertexPositions[0],
                                    vertexPositions[2] - vertexPositions[0]));
    
    const vec3 n2 = normalize(cross(vertexPositions[4] - vertexPositions[3],
                                    vertexPositions[5] - vertexPositions[3]));
    
    // Create some colors to represent those two normal vectors.
    const vec4 c1 = vec4(n1 * 0.5f + vec3(0.5f), 1.0f);
    const vec4 c2 = vec4(n2 * 0.5f + vec3(0.5f), 1.0f);
    
    // AFOX_TODO: Need real texture coordinates for faces here.
    
    // Pack vertex positions, colors, and texcoords together.
    std::array<TerrainVertex, n> terrainVertices = {{
        TerrainVertex(vec4(vertexPositions[0], 1.f), c1, texCoord),
        TerrainVertex(vec4(vertexPositions[1], 1.f), c1, texCoord),
        TerrainVertex(vec4(vertexPositions[2], 1.f), c1, texCoord),
        TerrainVertex(vec4(vertexPositions[3], 1.f), c2, texCoord),
        TerrainVertex(vec4(vertexPositions[4], 1.f), c2, texCoord),
        TerrainVertex(vec4(vertexPositions[5], 1.f), c2, texCoord)
    }};
    
    return terrainVertices;
}

void MesherNaiveSurfaceNets::emitFace(StaticMesh &geometry,
                                      const Array3D<Voxel> &voxels,
                                      float isosurface,
                                      const AABB &cell,
                                      size_t face)
{
    const auto vertices = verticesForFace(voxels, isosurface, cell, face);
    geometry.addVertices(vertices);
}

StaticMesh MesherNaiveSurfaceNets::extract(const Array3D<Voxel> &voxels,
                                           const AABB &aabb,
                                           float level)
{
    static const std::array<vec3, NUM_FACES> offsets = {{
        vec3( 0.0f,  0.0f, +1.0f), // FRONT
        vec3(-1.0f,  0.0f,  0.0f), // LEFT
        vec3( 0.0f,  0.0f, -1.0f), // BACK
        vec3(+1.0f,  0.0f,  0.0f), // RIGHT
        vec3( 0.0f, +1.0f,  0.0f), // TOP
        vec3( 0.0f, -1.0f,  0.0f)  // BOTTOM
    }};
    
    const vec3 dim = voxels.cellDimensions();
    
    StaticMesh geometry;
    
    voxels.forEachCell(aabb, [&](const AABB &cell){
        for (size_t i = 0; i < NUM_FACES; ++i) {
            const Voxel &thisVoxel = voxels.get(cell.center);
            const Voxel &thatVoxel = voxels.get(cell.center + dim * offsets[i]);
            
            if ((thisVoxel.value < level) && (thatVoxel.value >= level)) {
                emitFace(geometry, voxels, level, cell, i);
            }
        }
    });
    
    return geometry;
}
