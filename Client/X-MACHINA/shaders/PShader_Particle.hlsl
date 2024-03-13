#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSParticle(GSOutput_Particle pin) : SV_TARGET
{
    return 1.f.xxxx;
}