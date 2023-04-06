cbuffer ShadowBuffer : register(b1)
{
     matrix shadowView;
};

float4 ToShadowScreenSpace(const in float4 vertex_world_pos)
{
    float4 shadowViewPos = mul(vertex_world_pos, shadowView);
    // bring the position from NDC space into texture coordinate
    return shadowViewPos * float4(0.5f, -0.5f, 1.0f, 1.0f) + float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowViewPos.w;
}