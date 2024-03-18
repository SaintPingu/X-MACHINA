#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSParticle(GSOutput_Particle pin) : SV_TARGET
{
    return gTextureMaps[gInputPraticles[pin.ID].TextureIndex].Sample(gsamLinearWrap, pin.UV) * gInputPraticles[pin.ID].FinalColor;
}