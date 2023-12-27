#include "PSResource.hlsl"
#include "VSResource.hlsl"

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};

struct VS_TEX_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float2 uv : UV;
};

#ifdef POST_PROCESSING
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTextureInstancing(VS_TEX_INSTANCING_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.cTexture = albedoTexture.Sample(samplerState, input.uv);
    output.distance = length(input.positionW - gf3CameraPosition);
    
    return output;
}
#else
float4 PSTextureInstancing(VS_TEX_INSTANCING_OUTPUT input) : SV_TARGET
{
    return albedoTexture.Sample(samplerState, input.uv);
}
#endif