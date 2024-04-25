#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSScrollSmoke(GSOutput_Particle pin) : SV_TARGET
{
    float2 screenUV = float2(pin.PosH.x / gPassCB.FrameBufferWidth, pin.PosH.y / gPassCB.FrameBufferHeight);
    float depthViewZ = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, screenUV + gPassCB.TotalTime, 0.f).x);
    float particleViewZ = NdcDepthToViewDepth(pin.PosH.z);
    
    float depthDiff = depthViewZ - particleViewZ;
    
    if (depthDiff < 0)
        discard;
    
    float depthFade = saturate(pow(depthDiff, 2.f));
    
    float2 scrollUV = float2(pin.UV.x, pin.UV.y - gInputPraticles[pin.ID].CurTime / 2.f);
    float alphaMask = GammaDecoding(gTextureMaps[gInputPraticles[pin.ID].TextureIndex].SampleLevel(gsamLinearWrap, pin.UV, 0)).a * gInputPraticles[pin.ID].FinalColor.a;
    float4 color = GammaDecoding(gTextureMaps[gInputPraticles[pin.ID].TextureIndex].SampleLevel(gsamLinearWrap, scrollUV, 0));
    
    color.rgb *= gInputPraticles[pin.ID].FinalColor.xyz;
    color.a = alphaMask;
    
    color = lerp((0.5f, 0.5f, 0.5f, 0.5f), color, alphaMask);
    
    return color;
}