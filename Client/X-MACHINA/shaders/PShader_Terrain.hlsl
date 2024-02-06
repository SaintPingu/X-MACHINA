#include "Light.hlsl"

struct VSOutput_Terrain {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV0     : UVA;
    float2 UV1     : UVB;
};

struct PSOutput_MRT {
    float4 Texture  : SV_TARGET0;
    float  Distance : SV_TARGET4;
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
    
    float4 splatColor = gTextureMap[diffuseMap3Index].Sample(gsamLinearWrap, pin.UV1);
    float4 layer0 = float4(0, 0, 0, 0);
    float4 layer1 = float4(0, 0, 0, 0);
    float4 layer2 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.f)
    {
        layer0 = gTextureMap[diffuseMap0Index].Sample(gsamAnisotropicWrap, pin.UV0);
        layer0 *= splatColor.r;

    }
    if (splatColor.g > 0.f)
    {
        layer1 = gTextureMap[diffuseMap1Index].Sample(gsamAnisotropicWrap, pin.UV0);
        layer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        layer2 = gTextureMap[diffuseMap2Index].Sample(gsamAnisotropicWrap, pin.UV0);
        layer2 *= splatColor.b;
    }
    
    diffuseAlbedo *= normalize(layer0 + layer1 + layer2);
    if (gPassCB.FilterOption & Filter_Tone)
    {
        diffuseAlbedo = GammaDecoding(diffuseAlbedo);
    }
    
    float3 toCameraW = gPassCB.CameraPos - pin.PosW;
    float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;
    
    float3 shadowFactor  = 1.f;
    Material mat = { diffuseAlbedo, metallic, roughness };
    LightColor lightColor = ComputeLighting(mat, pin.PosW, pin.NormalW, toCameraW, shadowFactor);
    
    float4 litColor = ambient + float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    
    PSOutput_MRT output;
    output.Texture = litColor;
    output.Distance = length(toCameraW);
    
    return output;
}
