#include "Light.hlsl"

struct VSOutput_Water {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV : UV;
};

struct PSOutput_MRT {
    float4 Texture : SV_TARGET1;
    float  Distance : SV_TARGET4;
};


#ifdef POST_PROCESSING
PSOutput_MRT PSWater(VSOutput_Water input)
{
    PSOutput_MRT output;
    float4 color = gAlbedoTexture.Sample(gSamplerState, input.UV - float2(gDeltaTime * 0.06f, 0));
    float4 illumination = Lighting(input.PositionW, input.NormalW);
    
    output.Texture = lerp(color, illumination, 0.5f);
    output.Distance = input.PositionW;
    
    return output;
}
#else
float4 PSWater(VSOutput_Water input) : SV_TARGET
{
    float4 color = gAlbedoTexture.Sample(gSamplerState, input.UV - float2(gDeltaTime * 0.06f, 0));
    float4 illumination = Lighting(input.PositionW, input.NormalW);
    
    return lerp(color, illumination, 0.5f);
}
#endif