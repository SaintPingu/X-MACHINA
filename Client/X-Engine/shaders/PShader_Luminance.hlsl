#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSLuminance(VSOutput_Tex pin) : SV_TARGET
{
    float4 brightColor = (float4)0.f;
    float4 emissive = gTextureMaps[gPassCB.RT3G_EmissiveIndex].Sample(gsamPointWrap, pin.UV);
    
    float brightness = dot(emissive.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > 0.1f)
        brightColor = float4(emissive.rgb, 1.f);
    
    return brightColor;
}
