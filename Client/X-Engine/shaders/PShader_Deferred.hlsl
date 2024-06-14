#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

struct PSOutput_MRT {
    float4 Position           : SV_TARGET0;
    float4 Normal             : SV_TARGET1;
    float4 Diffuse            : SV_TARGET2;
    float4 Emissive           : SV_TARGET3;
    float2 MetallicSmoothness : SV_TARGET4;
    float1 Occlusion          : SV_TARGET5;
};

PSOutput_MRT PSDeferred(VSOutput_Standard pin)
{
    // material info
    MaterialInfo matInfo    = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse          = matInfo.Diffuse;
    float metallic          = 0;
    float roughness         = matInfo.Roughness;
    float occlusion         = 1.f;
    int diffuseMapIndex     = matInfo.DiffuseMap0Index;
    int normalMapIndex      = matInfo.NormalMapIndex;
    int metallicMapIndex    = matInfo.MetallicMapIndex;
    int emissiveMapIndex    = matInfo.EmissiveMapIndex;
    int occlusionMapIndex   = matInfo.OcclusionMapIndex;
    int occlusionMask       = matInfo.OcclusionMask;
    
    // sampling diffuseMap
    if (diffuseMapIndex != NONE) 
        diffuse *= GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));

    // sampling normalMap
    pin.NormalW = normalize(pin.NormalW);
    float3 bumpedNormalW = pin.NormalW;
    if (normalMapIndex != NONE) 
        bumpedNormalW = NormalSampleToWorldSpace(gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV).rgb, pin.NormalW, pin.TangentW);
    
    // sampling emissiveMap
    float4 emissiveMapSample = (float4)0;
    if (emissiveMapIndex != NONE) 
        emissiveMapSample = gTextureMaps[emissiveMapIndex].Sample(gsamAnisotropicWrap, pin.UV);

    // sampling metallicMap
    float4 metallicMapSample = (float4)0;
    if (metallicMapIndex != NONE)
    {
        metallicMapSample = GammaDecoding(gTextureMaps[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        metallic = metallicMapSample.r;
        roughness = 1 - metallicMapSample.a;
    }
    
    // sampling occlusionMap
    if (occlusionMapIndex != NONE) 
        occlusion = (float)GammaDecoding(gTextureMaps[occlusionMapIndex].Sample(gsamAnisotropicWrap, pin.UV).x);
    
    // apply rim light
    float4 hitRimLight = ComputeRimLight(float4(gObjectCB.HitRimColor, 1.f), 0.6f, gObjectCB.HitRimFactor, pin.PosW, bumpedNormalW);
    float4 mindRimLight = ComputeRimLight(float4(gObjectCB.MindRimColor, 1.f),0.6f, gObjectCB.MindRimFactor, pin.PosW, bumpedNormalW);

    // apply occlusion mask
    if (occlusionMask == TRUE) 
        ApplyOcculsionMaskByCamera(pin.PosW, pin.UV);
    
    // lit color
    PSOutput_MRT pout;
    pout.Position = float4(pin.PosW, 0.f);
    pout.Normal = float4(bumpedNormalW, 0.f);
    pout.Diffuse = diffuse + hitRimLight + mindRimLight;
    pout.Emissive = emissiveMapSample;
    pout.MetallicSmoothness = float2(metallic, roughness);
    pout.Occlusion = occlusion;
    
    return pout;
}
