const float3 normals[] = {
    float3(0.0, 0.0, 1.0),
    float3(0.0, 0.0, -1.0),
    float3(1.0, 0.0, 0.0),
    float3(-1.0, 0.0, 0.0),
    float3(0.0, -1.0, 0.0),
    float3(0.0, 1.0, 0.0)
};

SamplerState u_sampler : register(s1, space0);
Texture2D<float4> u_atlas : register(t2, space0);

struct Input
{
    centroid float2 texcoord : TEXCOORD0;
    float face : TEXCOORD1;
};

struct Output
{
    float4 color : SV_Target0;
};

Output main(Input input)
{
    float4 tex_color = u_atlas.Sample(u_sampler, input.texcoord);

    if (tex_color.a < 0.1f)
    {
        discard;
    }

    float ndotl = max(dot(normals[uint(input.face)], float3(0.0, 1.0, 0.0)), 0.5);

    Output output;
    output.color = float4(ndotl * tex_color.rgb, 1.0);
    return output;
}
