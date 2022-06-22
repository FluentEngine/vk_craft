SamplerState u_sampler : register(s0, space0);
Texture2D<float4> u_atlas : register(t1, space0);

struct Input
{
    float2 texcoord : TEXCOORD0;
};

struct Output
{
    float4 color : SV_Target0;
};

Output main(Input input)
{
    float4 color = u_atlas.Sample(u_sampler, input.texcoord);
    if (color.a < 0.1f)
    {
        discard;
    }

    Output output;
    output.color = float4(color.rgb, 1.0f);
    return output;
}
