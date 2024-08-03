#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSNightVisionAbility(VSOutput_Tex pin) : SV_TARGET0
{
    float4 color = gTextureMaps[gPassCB.RT0O_OffScreenIndex].Sample(gsamAnisotropicWrap, pin.UV);

    // ��� ���� (RGB�� ȸ������ ��ȯ)
    float brightness = dot(color.rgb, float3(0.299, 0.587, 0.114));

    // �߰� ���ð� ���� ���� (��� ��)
    float3 nightVisionColor = float3(0.1, 0.9f, 0.1);

    // ��� ����
    brightness = saturate(brightness * 2.0); // ��� ����, �ʿ信 ���� ���� ����
    
    // Noise //
    float noiseStrength = 0.1f;
    NumberGenerator rand;
    rand.SetSeed((uint) (gAbilityCB.AccTime * 10000));
    float2 uv = pin.UV + rand.GetRandomFloat(0, 10000);
    float4 noiseSample = gTextureMaps[gAbilityCB.NoiseIndex].Sample(gsamAnisotropicWrap, uv);
    float3 noise = (noiseSample.rgb - 0.5f) * noiseStrength;
    
    float4 finalColor = float4((nightVisionColor * brightness) + noise, 1.0);
    
    float lifeRatio = gAbilityCB.AccTime / gAbilityCB.ActiveTime;
    float startBandRatio = CalcAmplification(gAbilityCB.AccTime, 0.f, gAbilityCB.ActiveTime - gAbilityCB.Duration);
    float endBandRatio = CalcAttenuation(gAbilityCB.AccTime, gAbilityCB.Duration, gAbilityCB.ActiveTime);
    
    float startBand = GeneratedBand(pin.UV, false, 1.f, 2.f * startBandRatio, 0.7f);
    float endBand = GeneratedBand(pin.UV, false, 1.f, 2.f * endBandRatio, 0.7f);
    float litBand = lifeRatio < 0.5f ? startBand : endBand;

    // �׵θ� ��Ӱ�
    float2 center = float2(0.5f, 0.5f);
    float distanceFromCenter = length(pin.UV - center);
    float distanceFactor = pow(1.0 - distanceFromCenter, 3.0);
    
    return finalColor * litBand * distanceFactor;
}
