#include "Common.hlsl"

struct VSInput_TexInst {
    float3 PosL : POSITION;
    float2 UV   : UV;
};

struct VSOutput_TexInst {
    float4 PosH : SV_POSITION;
    float3 PosW : POSITIONW;
    float2 UV   : UV;
};

VSOutput_TexInst VSTextureInstancing(VSInput_TexInst input, uint nInstanceID : SV_InstanceID)
{
    VSOutput_TexInst output;
    output.PosW = mul(float4(input.PosL, 1.f), colorInstBuffer[nInstanceID].MtxObject).xyz;
    output.PosH = mul(mul(float4(output.PosW, 1.f), gPassCB.MtxView), gPassCB.MtxProj);
    output.UV = input.UV;
    return output;
}
