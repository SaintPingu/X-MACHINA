#include "VSResource.hlsl"

struct VSInput_Skybox {
    float3 Position : POSITION;
};

struct VSOutput_Skybox {
    float3 PositionL : POSITION;
    float4 Position : SV_POSITION;
};


VSOutput_Skybox VSSkyBox(VSInput_Skybox input)
{
    VSOutput_Skybox output;

    output.Position = mul(mul(mul(float4(input.Position, 1.f), gWorld), gMtxView), gMtxProj).xyww;
    output.PositionL = input.Position;

    return output;
}