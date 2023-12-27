#include "VSResource.hlsl"

struct VS_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float4 color : COLOR;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};

#ifdef POST_PROCESSING
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSInstancing(VS_INSTANCING_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.cTexture = input.color;
    output.distance = length(input.positionW - gf3CameraPosition);
    
    return output;
}
#else
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
    return input.color;
}
#endif
