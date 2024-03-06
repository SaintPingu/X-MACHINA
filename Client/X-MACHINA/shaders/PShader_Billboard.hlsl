#include "Common.hlsl"

struct VSOutput_Billboard {
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 UV   : UV;
};

float4 PSBillboard(VSOutput_Billboard input) : SV_TARGET
{
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuseAlbedo = gTextureMaps[mat.DiffuseMap0Index].Sample(gsamAnisotropicWrap, input.UV);
    
    if (gPassCB.FilterOption & Filter_Tone)
    {
        diffuseAlbedo = GammaDecoding(diffuseAlbedo);
    }
    
    if(diffuseAlbedo.a < 0.9f)
    {
        discard;
    }
    
    return diffuseAlbedo;
}
