#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSFinal(VSOutput_Tex pin) : SV_TARGET
{
    float4 color = gTextureMap[gPassCB.RT0_TextureIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float distance = gTextureMap[gPassCB.RT4_DistanceIndex].Sample(gsamAnisotropicWrap, pin.UV).x;

    //return FogDistance(color, distance);
    return color;
}
