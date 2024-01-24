#include "Common.hlsl"

struct VSInput_Water {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : UV;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct VSOutput_Water {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV : UV;
};

VSOutput_Water VSWater(VSInput_Water input)
{
    VSOutput_Water output;
    input.Position.y += sin(gDeltaTime * 1.35f + input.Position.x * 1.35f) * 1.95f + cos(gDeltaTime * 1.30f + input.Position.z * 1.35f) * 1.05f;

    output.PositionW = (float3) mul(float4(input.Position, 1.f), gMtxWorld);
    output.NormalW = mul(input.Normal, (float3x3) gMtxWorld);
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV * 5.0f;

    return output;
}