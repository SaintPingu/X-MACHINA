#include "Light.hlsl"

struct VSOutput_Water {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

float4 PSWater(VSOutput_Water input) : SV_TARGET
{
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];

    float4 color = GammaDecoding(gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV - float2(gPassCB.DeltaTime * 0.06f, 0)));
    float4 illumination = Lighting(mat, input.PosW, input.NormalW);
    
    color = FogDistance(color, length(input.PosW - gPassCB.CameraPos));
    
    return GammaEncoding(color * illumination);

}