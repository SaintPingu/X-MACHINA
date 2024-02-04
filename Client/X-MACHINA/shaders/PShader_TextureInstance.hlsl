#include "Common.hlsl"

struct VSOutput_TexInst {
    float4 PosH : SV_POSITION;
    float3 PosW : POSITIONW;
    float2 UV   : UV;
};

float4 PSTextureInstancing(VSOutput_TexInst input) : SV_TARGET
{
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    float4 output = gTextureMap[mat.DiffuseMap0Index].Sample(gsamAnisotropicWrap, input.UV);
    
    return output;
}
