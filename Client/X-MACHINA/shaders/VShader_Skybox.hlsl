#include "Common.hlsl"

struct VSInput_Skybox {
    float3 PosL : POSITION;
};

struct VSOutput_Skybox {
    float3 PosL : POSITION;
    float4 PosH : SV_POSITION;
};

VSOutput_Skybox VSSkyBox(VSInput_Skybox input)
{
    VSOutput_Skybox output;

    output.PosH = mul(mul(mul(float4(input.PosL, 1.f), gObjectCB.MtxWorld), gPassCB.MtxView), gPassCB.MtxProj).xyww;
    output.PosL = input.PosL;
    
    return output;
}