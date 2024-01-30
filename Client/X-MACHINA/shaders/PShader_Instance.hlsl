#include "Common.hlsl"

struct VSOutput_Inst {
    float4 PosH  : SV_POSITION;
    float3 PosW  : POSITIONW;
    float4 Color : COLOR;
};

struct PSOutput_MRT {
    float4 Texture : SV_TARGET0;
    float  Distance : SV_TARGET4;
};

PSOutput_MRT PSInstancing(VSOutput_Inst input)
{
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    
    PSOutput_MRT output;
    //output.Texture = mat.Diffuse; // 해당 값이 실제 모델의 머티리얼 정보 값이다.
    output.Texture = input.Color;
    output.Distance = input.PosW;
    
    return output;
}