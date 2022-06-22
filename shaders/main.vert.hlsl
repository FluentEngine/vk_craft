cbuffer global_ubo : register(b0)
{
    float4x4 projection;
    float4x4 view;
};

struct Input
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float face : TEXCOORD1;
};

struct Output
{
    float2 texcoord : TEXCOORD0;
    float face : TEXCOORD1;
    float4 position : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.position = mul(projection, mul(view, float4(input.position, 1.0f)));
    output.texcoord = input.texcoord;
    output.face = input.face;
    return output;
}
