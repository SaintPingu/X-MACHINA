#include "Common.hlsl"

struct VSInput_Particle {
    float3 PosL : POSITION;
    float3 UV   : UV;
    uint   ID   : SV_InstanceID;
};

struct VSOutput_Particle
{
    float3 PosW : POSITION;
    float2 UV   : UV;
    uint   ID   : ID;
};

VSOutput_Particle VSParticle(VSInput_Particle vin)
{
    VSOutput_Particle vout;

	float4 posW = mul(float4(vin.PosL, 1.0f), gObjectCB.MtxWorld);
    vout.PosW = posW.xyz;   
    vout.PosW += gInputPraticles[vin.ID].WorldPos;
    vout.ID = vin.ID;
    
    return vout;
}