#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

struct PSAerialControllerOutput
{
    float4 color : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Emissive : SV_TARGET2;
};

PSAerialControllerOutput PSAerialController(VSOutput_Tex pin)
{
    PSAerialControllerOutput pout;
    
    float3 normalW = gTextureMaps[gPassCB.RT1G_NormalIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;
    float3 posW = gTextureMaps[gPassCB.RT0G_PositionIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;

    float sceneDepth = gTextureMaps[gPassCB.DefaultDsIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    float customDepth = gTextureMaps[gPassCB.CustomDsIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    
    float rimWidth = 1.4f;
    float rim = 1.0f - max(0, abs(dot(normalW, normalize(gPassCB.CameraPos - posW))));
    rim = smoothstep(1.0f - rimWidth, 1.0f, rim);
    
    float4 litColor = (float4) 0;
    
    if (sceneDepth < customDepth)
    {
        float4 color1 = float4(0.6f, 0.6f, 0.6f, 1.f);
        float4 color2 = float4(0.3f, 0.3f, 0.3f, 1.f);
        litColor = lerp(color1, color2, rim);
    }
    else
    {
        float4 color1 = float4(0.05f, 0.05f, 0.05f, 1.f);
        float4 color2 = float4(0.1f, 0.1f, 0.1f, 1.f);
        litColor = lerp(color1, color2, rim);
    }

    // Noise //
    float noiseStrength = 0.04f;
    NumberGenerator rand;
    rand.SetSeed((uint) (gAbilityCB.AccTime * 1000));
    float2 noiseUV = pin.UV + rand.GetRandomFloat(0, 1000);
    float4 noiseSample = gTextureMaps[gAbilityCB.NoiseIndex].Sample(gsamAnisotropicWrap, noiseUV);
    litColor.rgb += noiseStrength * (noiseSample.rgb - 0.5f);
    
    float lifeRatio = gAbilityCB.AccTime / gAbilityCB.ActiveTime;
    float startBandRatio = CalcAmplification(gAbilityCB.AccTime, 0.f, gAbilityCB.ActiveTime - gAbilityCB.Duration);
    float endBandRatio = CalcAttenuation(gAbilityCB.AccTime, gAbilityCB.Duration, gAbilityCB.ActiveTime);
    
    float startBand = GeneratedBand(pin.UV, false, 1.f, 2.f * startBandRatio, 0.7f);
    float endBand = GeneratedBand(pin.UV, false, 1.f, 2.f * endBandRatio, 0.7f);
    float litBand = lifeRatio < 0.5f ? startBand : endBand;
    
    float4 offScreen = gTextureMaps[gPassCB.RT0O_OffScreenIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 ui = GammaDecoding(gTextureMaps[gAbilityCB.UIIndex].Sample(gsamAnisotropicWrap, pin.UV));
    float4 uiColor = (float4) 0;
    if (ui.a > 0.01f)
    {
        uiColor = ui;
    }

    litColor = (litColor * litBand) + (offScreen * 0.2f);
    float4 emissive = litColor * 0.3f;
    
    float4 noiseUI = GammaDecoding(gTextureMaps[gAbilityCB.UI2Index].Sample(gsamAnisotropicWrap, noiseUV));
    float t_noise = sin(gAbilityCB.AccTime);
    uiColor += (noiseUI * 0.01f * t_noise);
    
    float runTime = gAbilityCB.ActiveTime - gAbilityCB.AccTime;
    const float activateTime = 3.5f;
    if (runTime < activateTime)
    {
        float t_fullNose = (activateTime - runTime) / activateTime;
        uiColor = (uiColor * (1 - t_fullNose)) + (noiseUI * t_fullNose);
        litColor *= (1 - t_fullNose);
        emissive *= (1 - t_fullNose);
    }
    
    uiColor *= litBand;
    float4 finalColor = litColor + GammaEncoding(uiColor);
    
    pout.color = finalColor;
    pout.Normal = (float4) 0;
    pout.Emissive = (float4) 0;
    if (litColor.x > 0.3f)
    {
        pout.Emissive = emissive;
    }
    
    return pout;
}
