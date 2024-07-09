#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSOffScreen(VSOutput_Tex pin) : SV_TARGET
{
    float4 color = gTextureMaps[gPostPassCB.RT0_OffScreenIndex].Sample(gsamLinearClamp, pin.UV);

    float1 outlineFactor = gTextureMaps[gPostPassCB.OutlineIndex].Sample(gsamLinearClamp, pin.UV).x;
    float4 outlineColor = float4(1.f, 1.f, 1.f, 1.f) * outlineFactor;
    
    return color + outlineColor;
}
