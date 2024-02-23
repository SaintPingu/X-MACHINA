#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSFinal(VSOutput_Tex pin) : SV_TARGET
{
    float4 diffuseAlbedo = gTextureMap[gPassCB.RT0L_DiffuseIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 specularAlbedo = gTextureMap[gPassCB.RT1L_SpecularIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;

    //float distance = gTextureMap[gPassCB.RT4_DistanceIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    //return FogDistance(color, distance);

    float4 litColor = ambient + GammaEncoding(diffuseAlbedo) + GammaEncoding(specularAlbedo);
    
    return ((gPassCB.FilterOption & Filter_Tone) ? GammaDecoding(litColor) : litColor);
}
