#include "Common.hlsl"

struct VSInput_Shadow
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
};

void PSShadow(VSInput_Shadow pin)
{
}