#include "Common.hlsl"

struct VSOutput_Billboard {
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 UV   : UV;
};

float4 PSBillboard(VSOutput_Billboard input) : SV_TARGET
{
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    float4 color = float4(gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV));
    if(color.a < 0.9f)
    {
        discard;
    }
    
    return color;
}
