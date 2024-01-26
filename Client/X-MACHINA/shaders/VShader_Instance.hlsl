#include "Common.hlsl"

struct VSInput_Inst {
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VSOutput_Inst {
    float4 PosH  : SV_POSITION;
    float3 PosW  : POSITIONW;
    float4 Color : COLOR;
};


VSOutput_Inst VSInstancing(VSInput_Inst input, uint instanceID : SV_InstanceID)
{
    VSOutput_Inst output;
    output.PosW = mul(float4(input.PosL, 1.f), colorInstBuffer[instanceID].MtxObject).xyz;
    output.PosH = mul(mul(float4(output.PosW, 1.f), gMtxView), gMtxProj);
    output.Color = colorInstBuffer[instanceID].Color;
    
    return output;
}
