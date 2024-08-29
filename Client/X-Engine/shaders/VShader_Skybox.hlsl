#include "Common.hlsl"

struct VSInput_Skybox {
    float3 PosL : POSITION;
};

struct VSOutput_Skybox {
    float3 PosL : POSITION;
    float4 PosH : SV_POSITION;
};

VSOutput_Skybox VSSkyBox(VSInput_Skybox vin)
{
    VSOutput_Skybox vout;

	vout.PosL = vin.PosL;
    
	float4 posW = mul(float4(vin.PosL, 1.0f), gObjectCB.MtxWorld);
    
	posW.xyz += gPassCB.CameraPos;
    //posW.xyz += float3(0.f, 0.15f, 0.1f);
    
    vout.PosH = mul(mul(posW, gPassCB.MtxView), gPassCB.MtxProj).xyww;
    
    return vout;
}