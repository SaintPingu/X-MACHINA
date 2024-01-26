#include "Light.hlsl"

struct VSOutput_Water {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

float4 PSWater(VSOutput_Water input) : SV_TARGET
{
    MaterialInfo mat = materialBuffer[gMatIndex];

    float4 color = gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV - float2(gDeltaTime * 0.06f, 0));
    float4 illumination = Lighting(mat, input.PosW, input.NormalW);
    
    color = FogDistance(color, length(input.PosW - gCameraPos));
    
    return lerp(color, illumination, 0.5f);
}