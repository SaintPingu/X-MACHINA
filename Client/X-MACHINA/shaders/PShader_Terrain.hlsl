#include "Light.hlsl"

struct VSOutput_Terrain {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

struct PSOutput_MRT {
    float4 Position           : SV_TARGET0;
    float4 Normal             : SV_TARGET1;
    float4 Diffuse            : SV_TARGET2;
    float4 Emissive           : SV_TARGET3;
    float2 MetallicSmoothness : SV_TARGET4;
};

PSOutput_MRT PSTerrain(VSOutput_Terrain pin)
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuseAlbedo = matInfo.Diffuse;
    float metallic       = matInfo.Metallic;
    float roughness      = matInfo.Roughness;
    int diffuseMap0Index = matInfo.DiffuseMap0Index;
    int diffuseMap1Index = matInfo.DiffuseMap1Index;
    int diffuseMap2Index = matInfo.DiffuseMap2Index;
    int diffuseMap3Index = matInfo.DiffuseMap3Index;
    
    pin.NormalW = normalize(pin.NormalW);
    
    float corr = (1.f / 512.f);
    float2 uv1 = (pin.UV * 10) * corr;
    
    float4 splatColor = gTextureMap[diffuseMap3Index].Sample(gsamLinearWrap, uv1);
    float4 layer0 = float4(0, 0, 0, 0);
    float4 layer1 = float4(0, 0, 0, 0);
    float4 layer2 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.f)
    {
        layer0 = gTextureMap[diffuseMap0Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer0 *= splatColor.r;

    }
    if (splatColor.g > 0.f)
    {
        layer1 = gTextureMap[diffuseMap1Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        layer2 = gTextureMap[diffuseMap2Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer2 *= splatColor.b;
    }
    
    diffuseAlbedo *= normalize(layer0 + layer1 + layer2);
    diffuseAlbedo = GammaDecoding(diffuseAlbedo);

    PSOutput_MRT pout;
    pout.Position = float4(pin.PosW, 0.f);
    pout.Normal = float4(pin.NormalW, 0.f);
    pout.Diffuse = diffuseAlbedo;
    pout.Emissive = float4(0.f, 0.f, 0.f, 0.f);
    pout.MetallicSmoothness = float2(metallic, roughness);
    
    return pout;
}
