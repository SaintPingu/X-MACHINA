#include "Common.hlsl"

struct VSInput_Canvas {
    float3 PosW : POSITION;
    float2 UV   : UV;
};

struct VSOutput_Canvas {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

VSOutput_Canvas VSCanvas(VSInput_Canvas vin)
{
    VSOutput_Canvas vout;
    
    vout.PosH = float4(vin.PosW * 2.f, 1.f);
    vout.UV = vin.UV;
    
    vout.PosH = mul(vout.PosH, gObjectCB.MtxWorld);
    vout.UV = mul(float3(vout.UV, 1.f), (float3x3) (gObjectCB.MtxSprite)).xy;
    
    return vout;
}