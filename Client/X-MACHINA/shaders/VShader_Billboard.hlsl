#include "Common.hlsl"

struct VSInput_Standard {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : UV;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct VSOutput_Billboard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float2 UV : UV;
};



VSOutput_Billboard VSBillboard(VSInput_Standard input)
{
    VSOutput_Billboard output;
    
    output.PositionW = mul(float4(input.Position, 1.f), gMtxWorld).xyz;
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV;
    
    return output;
}

VSOutput_Billboard VSSprite(VSInput_Standard input)
{
    VSOutput_Billboard output;

    output.PositionW = mul(float4(input.Position, 1.f), gMtxWorld).xyz;
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = mul(float3(input.UV, 1.f), (float3x3) (gMtxSprite)).xy;

    return output;
}