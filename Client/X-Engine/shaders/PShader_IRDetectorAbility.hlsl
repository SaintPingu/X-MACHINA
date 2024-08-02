#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSIRDetectorAbility(VSOutput_Tex pin) : SV_TARGET0
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
    
    // Noise //
    float noiseStrength = 0.1f;
    NumberGenerator rand;
    rand.SetSeed((uint) (gAbilityCB.AccTime * 10));
    float2 uv = pin.UV + rand.GetRandomFloat(0, 100);
    float4 noiseSample = gTextureMaps[gAbilityCB.NoiseIndex].Sample(gsamAnisotropicWrap, uv);
    litColor.rgb += noiseStrength * (noiseSample.rgb - 0.5f);
    
    float lifeRatio = gAbilityCB.AccTime / gAbilityCB.ActiveTime;
    float startBandRatio = CalcAmplification(gAbilityCB.AccTime, 0.f, gAbilityCB.ActiveTime - gAbilityCB.Duration);
    float endBandRatio = CalcAttenuation(gAbilityCB.AccTime, gAbilityCB.Duration, gAbilityCB.ActiveTime);
    
    float startBand = GeneratedBand(pin.UV, false, 1.f, 2.f * startBandRatio, 0.7f);
    float endBand = GeneratedBand(pin.UV, false, 1.f, 2.f * endBandRatio, 0.7f);
    float litBand = lifeRatio < 0.5f ? startBand : endBand;
    
    float4 offScreen = gTextureMaps[gPassCB.RT0O_OffScreenIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 ui = GammaDecoding(gTextureMaps[gAbilityCB.UIIndex].Sample(gsamAnisotropicWrap, pin.UV));
    float4 uiColor = (float4)0;
    
    // alpha test and check red
    if (ui.a > 0.9f)
    {
        uiColor = ui * litBand;
        
        // flickering rec ui
        float uiRecSpeed = 5.f;
        if (ui.r >= 0.8f && ui.g <= 0.1f && ui.b <= 0.1f)
        {
            uiColor.xyz *= sin(gAbilityCB.AccTime * uiRecSpeed) * 0.5f + 0.5f;
        }   
    }

    litColor = litColor * litBand + (offScreen * (1 - litBand));
    litColor += GammaEncoding(uiColor);
    
    return litColor;
}
