#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSOffScreen(VSOutput_Tex pin) : SV_TARGET
{
    float4 color = gTextureMaps[gPassCB.RT1G_NormalIndex].Sample(gsamLinearClamp, pin.UV);

    return color;
}
