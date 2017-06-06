//
//  MesherBlocky.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/5/17.
//
//

#include "Terrain/MesherBlocky.hpp"
#include "Renderer/TerrainVertex.hpp"

using namespace glm;

static constexpr float L = 0.5f;
static const vec3 LLL(L, L, L);

std::array<glm::vec3, 4> MesherBlocky::quadForFace(const AABB &cell, size_t i)
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
    
    return quad[i];
}

std::array<vec3, 6> MesherBlocky::verticesForFace(const AABB &cell, size_t face)
{
    const std::array<glm::vec3, 4> quad = quadForFace(cell, face);
    
    // Stitch vertices of the quad together into two triangles.
    constexpr size_t n = 6;
    std::array<vec3, n> vertices;
    constexpr size_t indices[n] = { 0, 1, 2, 0, 2, 3 };
    for (size_t i = 0; i < n; ++i) {
        vertices[i] = cell.center + cell.extent * quad[indices[i]];
    }
    
    return vertices;
}

void MesherBlocky::emitVertex(StaticMesh &geometry,
                              const glm::vec3 &p,
                              const glm::vec4 &color)
{
    geometry.addVertex(TerrainVertex(vec4(p.x, p.y, p.z, 1.f),
                                     color,
                                     vec3(0.0f, 0.0f, 0.0f)));
}

void MesherBlocky::emitFace(StaticMesh &geometry, const AABB &cell, size_t i)
{
    static const std::array<vec4, NUM_FACES> color = {{
        vec4(0.0f, 0.0f, 1.0f, 1.0f), // FRONT
        vec4(0.0f, 1.0f, 0.0f, 1.0f), // LEFT
        vec4(0.0f, 0.0f, 1.0f, 1.0f), // BACK
        vec4(0.0f, 1.0f, 0.0f, 1.0f), // RIGHT
        vec4(1.0f, 1.0f, 1.0f, 1.0f), // TOP
        vec4(1.0f, 1.0f, 1.0f, 1.0f), // BOTTOM
    }};
    
    std::array<vec3, NUM_FACES> quad = verticesForFace(cell, i);
    
    for (const vec3 &p : quad) {
        emitVertex(geometry, p, color[i]);
    }
}

StaticMesh MesherBlocky::extract(const Array3D<Voxel> &voxels,
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
                emitFace(geometry, cell, i);
            }
        }
    });
    
    return geometry;
}
