#include "Common.hlsl"

struct VSInput_TexInst {
    float3 Position : POSITION;
    float2 UV : UV;
};

struct VSOutput_TexInst {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float2 UV : UV;
};

VSOutput_TexInst VSTextureInstancing(VSInput_TexInst input, uint nInstanceID : SV_InstanceID)
{
    VSOutput_TexInst output;
    output.PositionW = mul(float4(input.Position, 1.f), colorInstBuffer[nInstanceID].MtxObject).xyz;
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.UV = input.UV;
    return output;
}
