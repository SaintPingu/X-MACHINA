#include "Common.hlsl"

struct GSOutput_Particle
{
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

float4 PSParticle(GSOutput_Particle pin) : SV_TARGET
{
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    
    float4 color = gTextureMaps[matInfo.DiffuseMap0Index].Sample(gsamLinearWrap, pin.UV);
    color.a *= 1 - abs((gInputPraticles[pin.ID].CurTime / gInputPraticles[pin.ID].LifeTime) * 2 - 1);
    
    return color;
}