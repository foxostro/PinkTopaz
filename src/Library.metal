using namespace metal;

struct Vertex
{
    float4 position [[position]];
};

vertex Vertex vert(constant float4 *position [[buffer(0)]],
                   uint vid [[vertex_id]])
{
    Vertex vert;
    vert.position = position[vid];
    return vert;
}

fragment float4 frag(Vertex vert [[stage_in]])
{
    return float4(1.0, 0.3, 0.0, 1.0);
}

vertex Vertex text_vert(constant float4 *position [[buffer(0)]],
                        uint vid [[vertex_id]])
{
    Vertex vert;
    vert.position = position[vid];
    return vert;
}

fragment float4 text_frag(Vertex vert [[stage_in]])
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
