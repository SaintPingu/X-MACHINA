#include "Light.hlsl"

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
    float3 positionL : POSITION;
    float4 position : SV_POSITION;
};

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
    float4 color = skyBoxTexture.Sample(samplerState, input.positionL);

    return lerp(color, fogColor, 0.9f);
}