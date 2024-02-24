#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSFinal(VSOutput_Tex pin) : SV_TARGET
{
    float4 diffuseAlbedo = gTextureMap[gPassCB.RT0L_DiffuseIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 specularAlbedo = gTextureMap[gPassCB.RT1L_SpecularIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 emissive = gTextureMap[gPassCB.RT3_EmissiveIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 ambient = gTextureMap[gPassCB.RT2L_AmbientIndex].Sample(gsamAnisotropicWrap, pin.UV);

    float4 litColor = ambient + emissive + diffuseAlbedo + specularAlbedo;
    
    return litColor;
}
