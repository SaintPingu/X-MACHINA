#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSDownSampling(VSOutput_Tex pin) : SV_TARGET
{
    float4 downSample = gTextureMaps[gBloomCB.SamplingMapIndex].SampleLevel(gsamPointWrap, pin.UV, 0.f);
 
    return downSample;
}
