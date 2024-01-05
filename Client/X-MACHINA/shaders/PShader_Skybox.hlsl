#include "Light.hlsl"

struct VSOutput_Skybox {
    float3 PositionL : POSITION;
    float4 Position : SV_POSITION;
};

float4 PSSkyBox(VSOutput_Skybox input) : SV_TARGET
{
    float4 color = gSkyBoxTexture.Sample(gSamplerState, input.PositionL);

    return lerp(color, gFogColor, 0.9f);
}