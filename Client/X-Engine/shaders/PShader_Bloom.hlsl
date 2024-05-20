#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSBloom(VSOutput_Tex pin) : SV_TARGET
{
    float4 brightColor = (float4)0.f;
    float4 offScreen = gTextureMaps[gPassCB.RT0O_OffScreenIndex].SampleLevel(gsamPointWrap, pin.UV, 0.f);
    
    float brightness = dot(offScreen.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > 0.99f)
        brightColor = float4(offScreen.rgb, 1.f);
    
    return brightColor;
}
