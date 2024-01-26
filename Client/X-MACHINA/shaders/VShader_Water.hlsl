#include "Common.hlsl"

struct VSInput_Water {
    float3 PosL       : POSITION;
    float3 NormalL    : NORMAL;
    float2 UV         : UV;
    float3 TangentL   : TANGENT;
    float3 BiTangentL : BITANGENT;
};

struct VSOutput_Water {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

VSOutput_Water VSWater(VSInput_Water input)
{
    VSOutput_Water output;
    input.PosL.y += sin(gDeltaTime * 1.35f + input.PosL.x * 1.35f) * 1.95f + cos(gDeltaTime * 1.30f + input.PosL.z * 1.35f) * 1.05f;

    output.PosW = (float3) mul(float4(input.PosL, 1.f), gMtxWorld);
    output.NormalW = mul(input.NormalL, (float3x3) gMtxWorld);
    output.PosH = mul(mul(float4(output.PosW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV * 5.0f;

    return output;
}