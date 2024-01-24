#include "Common.hlsl"

struct VSInput_Terrain {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV0 : UVA;
    float2 UV1 : UVB;
};

struct VSOutput_Terrain {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV0 : UVA;
    float2 UV1 : UVB;
};

VSOutput_Terrain VSTerrain(VSInput_Terrain input)
{
    VSOutput_Terrain output;

    output.PositionW = (float3) mul(float4(input.Position, 1.f), gMtxWorld);
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.NormalW = mul(input.Normal, (float3x3) gMtxWorld);
    output.UV0 = input.UV0;
    output.UV1 = input.UV1;

    return output;
}