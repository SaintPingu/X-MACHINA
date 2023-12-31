#include "PSResource.hlsl"
#include "VSResource.hlsl"

struct PSOutput_MRT {
    float4 Texture : SV_TARGET1;
    float  Distance : SV_TARGET4;
};

struct VSOutput_Billboard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float2 UV : UV;
};


#ifdef POST_PROCESSING
PSOutput_MRT PSBillboard(VSOutput_Billboard input)
{
    float4 color = float4(gAlbedoTexture.Sample(gSamplerState, input.UV));
    if(color.a < 0.9f)
    {
        discard;
    }
    
    PSOutput_MRT output;
    output.Texture = color;
    output.Distance = length(input.PositionW - gCameraPos);
    
    return output;
}
#else
float4 PSBillboard(VSOutput_Billboard input) : SV_TARGET
{
    float4 color = float4(gAlbedoTexture.Sample(gSamplerState, input.UV));
    if (color.a < 0.9f)
    {
        discard;
    }
    return color;
}
#endif