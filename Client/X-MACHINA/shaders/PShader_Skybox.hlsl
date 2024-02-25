#include "Common.hlsl"

struct VSOutput_Skybox {
    float3 PosL : POSITION;
    float4 PosH : SV_POSITION;
};

float4 PSSkyBox(VSOutput_Skybox pin) : SV_TARGET
{
    return gSkyBoxTexture.Sample(gsamAnisotropicWrap, pin.PosL);
    //return lerp(color, gPassCB.FogColor, 0.9f);
}