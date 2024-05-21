#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSUpSampling(VSOutput_Tex pin) : SV_TARGET
{
    float4 prevSample = gTextureMaps[gBloomCB.PrevTargetIndex].SampleLevel(gsamLinearClamp, pin.UV, 0.f);
    float4 baseSample = gTextureMaps[gBloomCB.BaseTargetIndex].SampleLevel(gsamLinearClamp, pin.UV, 0.f);
    
    return prevSample + baseSample;
}
