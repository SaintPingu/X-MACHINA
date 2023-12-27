#include "Light.hlsl"

struct VS_WATER_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : UV;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};


#ifdef POST_PROCESSING
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSWater(VS_WATER_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    float4 cColor = albedoTexture.Sample(samplerState, input.uv - float2(deltaTime * 0.06f, 0));
    float4 illumination = Lighting(input.positionW, input.normalW);
    
    output.cTexture = lerp(cColor, illumination, 0.5f);
    output.distance = input.positionW;
    
    return output;
}
#else
float4 PSWater(VS_WATER_OUTPUT input) : SV_TARGET
{
    float4 cColor = albedoTexture.Sample(samplerState, input.uv - float2(deltaTime * 0.06f, 0));
    float4 illumination = Lighting(input.positionW, input.normalW);
    
    return lerp(cColor, illumination, 0.5f);
}
#endif