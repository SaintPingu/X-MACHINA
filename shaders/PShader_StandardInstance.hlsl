#include "Light.hlsl"

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

float4 PS_Standard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    if (input.isTexture)
    {
        float4 cColor       = albedoTexture.Sample(samplerState, input.uv);
        float3 normalW      = normalize(input.normalW);
        float4 illumination = Lighting(input.positionW, normalW);
        
        return Fog(lerp(cColor, illumination, 0.5f), input.positionW);
    }
    else
    {
        float3 normalW = normalize(input.normalW);
        float4 color   = Lighting(input.positionW, normalW);
        return (color);
    }
}