#include "Light.hlsl"

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : UV;
    bool isTexture : ISTEXTURE;
};

#ifdef POST_PROCESSING

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    
    input.normalW = normalize(input.normalW);
    
    float4 illumination = Lighting(input.positionW, input.normalW);
    
    //output.position = float4(input.positionW, 1.f);
    //output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);
    output.distance = length(input.positionW - gf3CameraPosition);
    
    if (input.isTexture)
    {
        output.cTexture = albedoTexture.Sample(samplerState, input.uv) * illumination;
    }
    else
    {
        output.cTexture = illumination;
    }
    
    return (output);
}

#else
float4 PSTexturedLightingToMultipleRTs(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    return albedoTexture.Sample(samplerState, input.uv);
}

#endif