#include "Common.hlsl"

struct VSInput_Particle {
    float3 PosL : POSITION;
    uint   ID   : SV_InstanceID;
};

struct VSOutput_Particle
{
    float3 PosW : POSITION;
    uint   ID   : ID;
};

VSOutput_Particle VSParticle(VSInput_Particle vin)
{
    VSOutput_Particle vout;

	float4 posW = mul(float4(vin.PosL, 1.0f), gObjectCB.MtxWorld);
    vout.PosW = gInputPraticles[vin.ID].WorldPos;
    vout.ID = vin.ID;
    
    return vout;
}