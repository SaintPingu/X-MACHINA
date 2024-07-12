#include "Common.hlsl"

struct VSInput_Shadow
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
};

void PSShadow(VSInput_Shadow pin)
{
    MaterialInfo matInfo    = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse          = matInfo.Diffuse;
    int diffuseMapIndex     = matInfo.DiffuseMap0Index;
    
    // sampling diffuseMap
    if (diffuseMapIndex != NONE) 
         diffuse *= GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));

    if (matInfo.AlphaTest == TRUE)
        clip(diffuse.a - 0.1f);
}