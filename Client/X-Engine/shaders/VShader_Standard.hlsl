#include "Common.hlsl"

struct VSInput_Standard {
    float3 PosL       : POSITION;
    float3 NormalL    : NORMAL;
    float2 UV         : UV;
    float3 TangentL   : TANGENT;
    float3 BiTangentL : BITANGENT;
};

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

VSOutput_Standard VS_Standard(VSInput_Standard input)
{
    VSOutput_Standard output;

    output.PosW = (float3) mul(float4(input.PosL, 1.f), gObjectCB.MtxWorld);
    output.NormalW = mul(input.NormalL, (float3x3) gObjectCB.MtxWorld);
    output.TangentW = (float3) mul(float4(input.TangentL, 1.f), gObjectCB.MtxWorld);
    output.BiTangentW = (float3) mul(float4(input.BiTangentL, 1.f), gObjectCB.MtxWorld);
    output.PosH = mul(mul(float4(output.PosW, 1.f), gPassCB.MtxView), gPassCB.MtxProj);
    output.UV = input.UV;
    
    return output;
}