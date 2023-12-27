#include "VSResource.hlsl"

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


VS_OUTPUT VSWired(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);

    return output;
}
