#include "Common.hlsl"

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
};




VSOutput_Standard VS_StandardInstance(VSInput_Standard input, uint nInstanceID : SV_InstanceID)
{
    VSOutput_Standard output;

    matrix mtxWorld = instBuffer[nInstanceID].MtxObject;
    output.PositionW = mul(float4(input.Position, 1.f), mtxWorld).xyz;
    output.NormalW = mul(input.Normal, (float3x3) mtxWorld);
    output.TangentW = (float3) mul(float4(input.Tangent, 1.f), mtxWorld);
    output.BiTangentW = (float3) mul(float4(input.BiTangent, 1.f), mtxWorld);
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV;

    return output;
}