#include "Common.hlsl"

struct VSOutput_Billboard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float2 UV : UV;
};

float4 PSBillboard(VSOutput_Billboard input) : SV_TARGET
{
    MaterialInfo mat = materialBuffer[gMatIndex];
    float4 color = float4(gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV));
    if(color.a < 0.9f)
    {
        discard;
    }
    
    return color;
}
