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
};

struct TerrainUniforms
{
    float4x4 view, proj;
};

vertex TerrainProjectedVertex vert(TerrainVertex inVert [[stage_in]],
                                   constant TerrainUniforms &u [[buffer(1)]])
{
    TerrainProjectedVertex outVert;
    outVert.position = u.proj * u.view * inVert.vp;
    outVert.color = inVert.vc;
    outVert.texCoord = inVert.vt;
    return outVert;
}

fragment float4 frag(TerrainProjectedVertex vert [[stage_in]],
                     texture2d_array<float> diffuseTexture [[texture(0)]],
                     sampler textureSampler [[sampler(0)]])
{
    return vert.color * diffuseTexture.sample(textureSampler,
                                              vert.texCoord.xy,
                                              vert.texCoord.z);
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
    float r = diffuseTexture.sample(textureSampler, vert.texCoord).r;
    float4 texel = float4(vert.vertexColor.xyz, r);
    return texel;
}
