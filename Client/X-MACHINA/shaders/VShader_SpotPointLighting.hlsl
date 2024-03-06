#include "Common.hlsl"

struct VSInput_Lighting {
    float3 PosL : POSITION;
};

struct VSOutput_Lighting {
    float4 PosH : SV_POSITION;
};

VSOutput_Lighting VSSpotPointLighting(VSInput_Lighting vin)
{
    VSOutput_Lighting vout;
    
    float4 posW = mul(float4(vin.PosL, 1.0f), gObjectCB.MtxWorld);
    vout.PosH = mul(mul(posW, gPassCB.MtxView), gPassCB.MtxProj);

    return vout;
}