#include "Light.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSScreenRectSamplingTextured(VSOutput_Tex input) : SV_TARGET
{
    float4 color = gTextureMap[gPassCB.RT0_TextureIndex].Load(uint3((uint) input.PosH.x, (uint) input.PosH.y, 0));
    float4 white = float4(1.f, 1.f, 1.f, 1.f);
    
    float distance = gTextureMap[gPassCB.RT4_DistanceIndex].Load(uint3((uint) input.PosH.x, (uint) input.PosH.y, 0));
    color = FogDistance(color, distance);

    return color;
}
