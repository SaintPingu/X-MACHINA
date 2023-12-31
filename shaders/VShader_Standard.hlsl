#include "VSResource.hlsl"

struct VSInput_Standard {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : UV;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct VSOutput_Standard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
    bool IsTexture : ISTEXTURE;
};

VSOutput_Standard VS_Standard(VSInput_Standard input)
{
    VSOutput_Standard output;

    output.PositionW = (float3) mul(float4(input.Position, 1.f), gMtxWorld);
    output.NormalW = mul(input.Normal, (float3x3) gMtxWorld);
    output.TangentW = (float3) mul(float4(input.Tangent, 1.f), gMtxWorld);
    output.BiTangentW = (float3) mul(float4(input.BiTangent, 1.f), gMtxWorld);
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV;
    
    if (gTextureMask > 0)
    {
        output.IsTexture = true;
    }
    else
    {
        output.IsTexture = false;
    }

    return output;
}