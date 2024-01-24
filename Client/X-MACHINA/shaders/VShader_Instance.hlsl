#include "Common.hlsl"

struct VSInput_Inst {
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VSOutput_Inst {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float4 Color : COLOR;
};


VSOutput_Inst VSInstancing(VSInput_Inst input, uint instanceID : SV_InstanceID)
{
    VSOutput_Inst output;
    output.PositionW = mul(float4(input.Position, 1.f), colorInstBuffer[instanceID].MtxObject).xyz;
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.Color = colorInstBuffer[instanceID].Color;
    
    return output;
}
