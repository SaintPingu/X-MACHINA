#include "Common.hlsl"

struct VSInput_Standard {
    float3 PosL       : POSITION;
    float3 NormalL    : NORMAL;
    float2 UV         : UV;
    float3 TangentL   : TANGENT;
    float3 BiTangentL : BITANGENT;
};

struct VSOutput_Billboard {
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 UV   : UV;
};



VSOutput_Billboard VSBillboard(VSInput_Standard input)
{
    VSOutput_Billboard output;
    
    output.PosW = mul(float4(input.PosL, 1.f), gMtxWorld).xyz;
    output.PosH = mul(mul(float4(output.PosW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV;
    
    return output;
}

VSOutput_Billboard VSSprite(VSInput_Standard input)
{
    VSOutput_Billboard output;

    output.PosW = mul(float4(input.PosL, 1.f), gMtxWorld).xyz;
    output.PosH = mul(mul(float4(output.PosW, 1.f), gMtxView), gMtxProj);
    output.UV = mul(float3(input.UV, 1.f), (float3x3) (gMtxSprite)).xy;

    return output;
}