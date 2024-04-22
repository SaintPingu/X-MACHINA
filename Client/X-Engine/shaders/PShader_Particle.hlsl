#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSParticle(GSOutput_Particle pin) : SV_TARGET
{
    float2 screenUV = float2(pin.PosH.x / gPassCB.FrameBufferWidth, pin.PosH.y / gPassCB.FrameBufferHeight);
    float depthViewZ = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, screenUV, 0.f).x);
    float particleViewZ = NdcDepthToViewDepth(pin.PosH.z);
    
    float depthDiff = depthViewZ - particleViewZ;
    
    if (depthDiff < 0)
        discard;
    
    float depthFade = saturate(pow(depthDiff, 2.f));
    
    float4 color = gTextureMaps[gInputPraticles[pin.ID].TextureIndex].Sample(gsamLinearWrap, pin.UV) * gInputPraticles[pin.ID].FinalColor;
    color.a *= depthFade;
    
    return color;
}