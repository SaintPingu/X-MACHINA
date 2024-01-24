#include "Light.hlsl"

struct VSOutput_Standard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
};

float4 PS_Standard(VSOutput_Standard input) : SV_TARGET
{
    MaterialInfo mat = materialBuffer[gMatIndex];
    
    if (mat.DiffuseMap0Index != -1)
    {
        float4 color        = gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV);
        float3 normalW      = normalize(input.NormalW);
        float4 illumination = Lighting(mat, input.PositionW, normalW);
        
        return Fog(lerp(color, illumination, 0.5f), input.PositionW);
    }
    else 
    {
        float3 normalW = normalize(input.NormalW);
        float4 color   = Lighting(mat, input.PositionW, normalW);
        return color;
    }
}