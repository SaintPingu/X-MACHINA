#include "VSResource.hlsl"

struct VSInput {
    float3 Position : POSITION;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};


VSOutput VSWired(VSInput input)
{
    VSOutput output;

    output.Position = mul(mul(mul(float4(input.Position, 1.f), gMtxWorld), gMtxView), gMtxProj);
    output.Color = float4(1.f, 0.f, 0.f, 1.f);

    return output;
}
