#include "PSResource.hlsl"
#include "VSResource.hlsl"

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};

struct VS_BILLBOARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float2 uv : UV;
};


#ifdef POST_PROCESSING
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSBillboard(VS_BILLBOARD_OUTPUT input)
{
    float4 color = float4(albedoTexture.Sample(samplerState, input.uv));
    if(color.a < 0.9f)
    {
        discard;
    }
    
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.cTexture = color;
    output.distance = length(input.positionW - gf3CameraPosition);
    
    return output;
}
#else
float4 PSBillboard(VS_BILLBOARD_OUTPUT input) : SV_TARGET
{
    float4 color = float4(albedoTexture.Sample(samplerState, input.uv));
    if (color.a < 0.9f)
    {
        discard;
    }
    return color;
}
#endif