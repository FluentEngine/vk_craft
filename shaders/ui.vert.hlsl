#ifndef __hlsl_dx_compiler
[[vk::push_constant]]
#endif
cbuffer PushConstants
{
    float4 scale_offset;
};

struct Input
{
    float2 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct Output
{
    float2 texcoord : TEXCOORD0;
    float4 position : SV_Position;
};

Output main(Input input)
{
    float2 scale = scale_offset.rg;
    float2 offset = scale_offset.ba;
    float2 coord = input.position;
    coord.x *= scale.x;
    coord.y *= scale.y;
    coord.x += offset.x;
    coord.y += offset.y;

    Output output;
    output.texcoord = input.texcoord;
    output.position = float4(coord, 0.0f, 1.0f);
    return output;
}
