#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSIRDetectorAbility(VSOutput_Tex pin) : SV_TARGET
{
    float3 normalW = gTextureMaps[gPassCB.RT1G_NormalIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;
    float3 posW = gTextureMaps[gPassCB.RT0G_PositionIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;

    float sceneDepth = gTextureMaps[gPassCB.DefaultDsIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    float customDepth = gTextureMaps[gPassCB.CustomDsIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    
    float rimWidth = 1.4f;
    float rim = 1.0f - max(0, abs(dot(normalW, normalize(gPassCB.CameraPos - posW))));
    rim = smoothstep(1.0f - rimWidth, 1.0f, rim);
    
    float4 litColor = (float4)0;
    
    if (sceneDepth < customDepth)
    {
        float4 color1 = float4(1.f, 0.052f, 0.f, 1.f);
        float4 color2 = float4(0.336f, 1.f, 0.f, 1.f);
        litColor = lerp(color1, color2, rim);
    }
    else
    {
        float4 color1 = float4(0.00155f, 0.f, 0.0885f, 1.f);
        float4 color2 = float4(0.f, 0.0171f, 0.719f, 1.f);
        litColor = lerp(color1, color2, rim);
    }
    
    return litColor;
}
