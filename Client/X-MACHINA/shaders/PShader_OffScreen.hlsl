#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSOffScreen(VSOutput_Tex pin) : SV_TARGET
{
    float4 color = gTextureMap[gPostPassCB.RT0_OffScreenIndex].Sample(gsamAnisotropicWrap, pin.UV);

    return color;
}
