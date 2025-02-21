#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

struct PSOutput {
    float4 Result     : SV_TARGET0;
    float4 Normal     : SV_TARGET1;
    float4 Emissive   : SV_TARGET2;
};

PSOutput PSAfterSkinImage(VSOutput_Standard pin)
{
    // normalize normal
    pin.NormalW = normalize(pin.NormalW);

    // apply rim light
    float4 startColor = ComputeRimLight(float4(gObjectCB.HitRimColor, 1.f), 2.f, gObjectCB.HitRimFactor, pin.PosW, pin.NormalW);
    float4 endColor = ComputeRimLight(float4(1.f, 0.f, 0.f, 1.f), 2.f, gObjectCB.HitRimFactor, pin.PosW, pin.NormalW);
    
    PSOutput pout = (PSOutput)0;
    pout.Result = lerp(endColor, startColor, gObjectCB.HitRimFactor);
    pout.Normal = float4(pin.NormalW, 1.f);
    
    return pout;
}