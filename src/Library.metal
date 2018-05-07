#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;





struct TerrainVertex
{
    float4 vp [[attribute(0)]];
    float4 vc [[attribute(1)]];
    float3 vt [[attribute(2)]];
};

struct TerrainProjectedVertex
{
    float4 position [[position]];
    float4 color;
    float3 texCoord;
    float fogDensity;
};

struct TerrainUniforms
{
    float4x4 view, proj;
    float fogDensity;
};

vertex TerrainProjectedVertex vert(TerrainVertex inVert [[stage_in]],
                                   constant TerrainUniforms &u [[buffer(1)]])
{
    TerrainProjectedVertex outVert;
    outVert.position = u.proj * u.view * inVert.vp;
    outVert.color = inVert.vc;
    outVert.texCoord = inVert.vt;
    outVert.fogDensity = u.fogDensity;
    return outVert;
}

constant float4 fogColor = float4(0.2, 0.4, 0.5, 1.0);
constant float e = 2.71828182845904523536028747135266249;

fragment float4 frag(TerrainProjectedVertex vert [[stage_in]],
                     texture2d_array<float> diffuseTexture [[texture(0)]],
                     sampler textureSampler [[sampler(0)]])
{
    float4 baseColor = vert.color * diffuseTexture.sample(textureSampler,
                                                          vert.texCoord.xy,
                                                          vert.texCoord.z);
    
    // Apply a fog effect to the terrain geometry. Note the `2*' below. This
    // factor corrects for the difference between Metal's coordinate space for
    // normalized device coordinates and the one used by OpenGL. This is
    // necessary so that fog drawn with the Metal shader has the same density
    // as fog drawn with the GLSL shader.
    float depth = 2*vert.position.z / vert.position.w;
    float f = pow(e, -pow(depth * vert.fogDensity, 2.0));
    return mix(fogColor, baseColor, f);
}





struct WireframeCubeVertex
{
    float4 vp [[attribute(0)]];
};

struct WireframeCubeProjectedVertex
{
    float4 position [[position]];
    float4 color;
};

struct WireframeCubeUniforms
{
    float4x4 projection;
};

struct WireframeCubeUniformsPerInstance
{
    float4x4 modelView;
    float4 color;
};

vertex WireframeCubeProjectedVertex
wireframe_cube_vert(WireframeCubeVertex inVert [[stage_in]],
                    constant WireframeCubeUniforms &uniforms [[buffer(1)]],
                    constant WireframeCubeUniformsPerInstance *perInstanceUniforms [[buffer(2)]],
                    ushort instance [[instance_id]])
{
    WireframeCubeProjectedVertex outVert;
    outVert.position = uniforms.projection * perInstanceUniforms[instance].modelView * inVert.vp;
    outVert.color = perInstanceUniforms[instance].color;
    return outVert;
}

fragment float4
wireframe_cube_frag(WireframeCubeProjectedVertex vert [[stage_in]])
{
    return vert.color;
}





struct TextVertex
{
    float4 packed [[attribute(0)]]; // <vec2 pos, vec2 tex>
};

struct TextProjectedVertex
{
    float4 position [[position]];
    float2 texCoord;
    float4 vertexColor;
};

struct TextUniforms
{
    float4 textColor;
    float4x4 projection;
};

vertex TextProjectedVertex text_vert(TextVertex inVert [[stage_in]],
                                     constant TextUniforms &u [[buffer(1)]])
{
    TextProjectedVertex outVert;
    outVert.position = u.projection * float4(inVert.packed.xy, 0.0, 1.0);
    outVert.texCoord = inVert.packed.zw;
    outVert.vertexColor = u.textColor;
    return outVert;
}

fragment float4 text_frag(TextProjectedVertex vert [[stage_in]],
                          texture2d<float> diffuseTexture [[texture(0)]],
                          sampler textureSampler [[sampler(0)]])
{
    float4 texel = diffuseTexture.sample(textureSampler, vert.texCoord) * vert.vertexColor;
    return texel;
}
