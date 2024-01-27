#include "Light.hlsl"

struct VSOutput_Skybox {
    float3 PosL : POSITION;
    float4 PosH : SV_POSITION;
};

float4 PSSkyBox(VSOutput_Skybox input) : SV_TARGET
{
    float4 color = gSkyBoxTexture.Sample(gSamplerState, input.PosL);

    return lerp(color, gPassCB.FogColor, 0.9f);
}