#include "Common.hlsl"

struct VSInput_Terrain {
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 UV0     : UVA;
    float2 UV1     : UVB;
};

struct VSOutput_Terrain {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV0     : UVA;
    float2 UV1     : UVB;
};

VSOutput_Terrain VSTerrain(VSInput_Terrain input)
{
    VSOutput_Terrain output;

    output.PosW = (float3) mul(float4(input.PosL, 1.f), gObjectCB.MtxWorld);
    output.PosH = mul(mul(float4(output.PosW, 1.f), gPassCB.MtxView), gPassCB.MtxProj);
    output.NormalW = mul(input.NormalL, (float3x3) gObjectCB.MtxWorld);
    output.UV0 = input.UV0;
    output.UV1 = input.UV1;

    return output;
}