#include "Common.hlsl"

struct VSInput_SSAO {
    float3 PosL : POSITION;
    float2 UV   : UV;
};

struct VSOutput_SSAO {
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float2 UV   : UV;
};

VSOutput_SSAO VsSsao(VSInput_SSAO vin)
{
    VSOutput_SSAO vout;
    
    vout.PosH = float4(vin.PosL * 2.f, 1.f);
    vout.UV = vin.UV;
    
    // view 공간의 near 평면으로 변환
    float4 ph = mul(vout.PosH, gSsaoCB.MtxInvProj);
    vout.PosV = ph.xyz / ph.w;
    
    return vout;
}