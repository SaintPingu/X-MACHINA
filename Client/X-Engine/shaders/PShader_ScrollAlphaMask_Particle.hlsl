#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSScrollAlphaMaskParticle(GSOutput_Particle pin) : SV_TARGET
{
    float2 screenUV = float2(pin.PosH.x / gPassCB.FrameBufferWidth, pin.PosH.y / gPassCB.FrameBufferHeight);
    float depthViewZ = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, screenUV + gPassCB.TotalTime, 0.f).x);
    float particleViewZ = NdcDepthToViewDepth(pin.PosH.z);
    
    float depthDiff = depthViewZ - particleViewZ;
    
    if (depthDiff < 0)
        discard;
    
    float depthFade = saturate(pow(depthDiff, 2.f));
    
    float2 scrollUV = float2(pin.UV.x + gInputPraticles[pin.ID].CurTime, pin.UV.y);
    float alphaMask = gTextureMaps[gInputPraticles[pin.ID].TextureIndex].SampleLevel(gsamLinearWrap, pin.UV, 0).a;
    float4 color = gTextureMaps[gInputPraticles[pin.ID].TextureIndex].SampleLevel(gsamLinearWrap, scrollUV, 0) * gInputPraticles[pin.ID].FinalColor;

    return color * alphaMask * float4(1.f, 1.f, 1.f, depthFade);
}