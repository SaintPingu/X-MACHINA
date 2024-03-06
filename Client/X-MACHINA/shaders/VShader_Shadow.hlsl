#include "Common.hlsl"

struct VSInput_Shadow {
    float3 PosL : POSITION;
    float2 UV   : UV;
};

struct VSOutput_Shadow {
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

VSOutput_Shadow VSShadow(VSInput_Shadow vin)
{
    VSOutput_Shadow vout;
    
    float4 posW = mul(float4(vin.PosL, 1.f), gObjectCB.MtxWorld);
    vout.PosH = mul(mul(posW, gPassCB.MtxView), gPassCB.MtxProj);
    vout.UV = vin.UV;

    return vout;
}