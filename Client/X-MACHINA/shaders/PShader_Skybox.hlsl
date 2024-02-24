#include "Common.hlsl"

struct VSOutput_Skybox {
    float3 PosL : POSITION;
    float4 PosH : SV_POSITION;
};

float4 PSSkyBox(VSOutput_Skybox input) : SV_TARGET
{
    float4 diffuseAlbedo = gSkyBoxTexture.Sample(gsamLinearWrap, input.PosL);
    
    if (gPassCB.FilterOption & Filter_Tone)
    {
        diffuseAlbedo = (diffuseAlbedo);
    }
    
    //return lerp(color, gPassCB.FogColor, 0.9f);
    return diffuseAlbedo;
}