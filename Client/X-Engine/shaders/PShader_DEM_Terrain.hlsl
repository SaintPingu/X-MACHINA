#include "Light.hlsl"

struct VSOutput_Terrain {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

float4 PSDEMTerrain(VSOutput_Terrain pin) : SV_TARGET
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse       = matInfo.Diffuse;
    float metallic       = matInfo.Metallic;
    float roughness      = matInfo.Roughness;
    float occlusion      = 1.f;
    int diffuseMap0Index = matInfo.DiffuseMap0Index;
    int diffuseMap1Index = matInfo.DiffuseMap1Index;
    int diffuseMap2Index = matInfo.DiffuseMap2Index;
    int diffuseMap3Index = matInfo.DiffuseMap3Index;
    
    pin.NormalW = normalize(pin.NormalW);
    
    float2 uv1 = (pin.UV * 10);
    uv1.x *= (1.f / 1000.f);    // terrain width  [1000]
    uv1.y *= (1.f / 500.f);     // terrain height [500]
    
    float4 splatColor = gTextureMaps[diffuseMap3Index].Sample(gsamLinearWrap, uv1);
    float4 layer0 = float4(0, 0, 0, 0);
    float4 layer1 = float4(0, 0, 0, 0);
    float4 layer2 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.f)
    {
        layer0 = gTextureMaps[diffuseMap0Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer0 *= splatColor.r;

    }
    if (splatColor.g > 0.f)
    {
        layer1 = gTextureMaps[diffuseMap1Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        layer2 = gTextureMaps[diffuseMap2Index].Sample(gsamAnisotropicWrap, pin.UV);
        layer2 *= splatColor.b;
    }
    
    diffuse *= normalize(layer0 + layer1 + layer2);
    diffuse = diffuse;
    
    float3 toCameraW = normalize(gPassCB.CameraPos - pin.PosW);
    
    float3 diffuseAlbedo = lerp(diffuse.xyz, 0.0f, metallic);
    float3 specularAlbedo = lerp(0.03f, diffuse.xyz, metallic);
    Material mat = { diffuseAlbedo, specularAlbedo, metallic, roughness };
    LightColor lightColor = ComputeDirectionalLight(gPassCB.Lights[gObjectCB.LightIndex], mat, pin.PosW, pin.NormalW, toCameraW, 1.f);
    
    float4 litDiffuse = float4(lightColor.Diffuse, 1.f);
    float4 litSpecular = float4(lightColor.Specular, 1.f);
    float4 litColor = litDiffuse + litSpecular + diffuse * gPassCB.GlobalAmbient;

    return litColor;
}
