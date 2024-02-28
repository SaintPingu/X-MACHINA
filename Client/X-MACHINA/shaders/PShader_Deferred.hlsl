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
};

PSOutput_MRT PSDeferred(VSOutput_Standard pin)
{
    // material info
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse        = matInfo.Diffuse;
    float metallic        = matInfo.Metallic;
    float roughness       = matInfo.Roughness;
    int diffuseMapIndex   = matInfo.DiffuseMap0Index;
    int normalMapIndex    = matInfo.NormalMapIndex;
    int metallicMapIndex  = matInfo.MetallicMapIndex;
    int emissiveMapIndex  = matInfo.EmissiveMapIndex;
    int occlusionMapIndex = matInfo.OcclusionMapIndex;
    
    // sampling diffuseMap
    if (diffuseMapIndex != -1)
    {
        diffuse *= GammaDecoding(gTextureMap[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
    }

    // normalize normal
    pin.NormalW = normalize(pin.NormalW);
    float3 bumpedNormalW = pin.NormalW;
    
    // sampling normalMap, to world space
    float4 normalMapSample = (float4)0;
    if (normalMapIndex != -1)
    {
        normalMapSample = gTextureMap[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }
    
    // sampling emissiveMap
    float4 emissiveMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        emissiveMapSample = gTextureMap[emissiveMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }
    
    // sampling metallicMap
    float4 metallicMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        metallicMapSample = GammaDecoding(gTextureMap[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        metallic = metallicMapSample.r;
        roughness = 1 - metallicMapSample.a;
    }
    
    float4 occlusionMapSample = (float4)0;
    if (occlusionMapIndex != -1)
    {
        occlusionMapSample = gTextureMap[occlusionMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }
    
    float rimWidth = 0.8f;
    float gRimLightFactor = 0.4f;
    float4 gRimLightColor = float4(1.f, 1.f, 1.f, 0.f);
    float rim = 1.0f - max(0, dot(bumpedNormalW, normalize(gPassCB.CameraPos - pin.PosW)));
    rim = smoothstep(1.0f - rimWidth, 1.0f, rim) * gRimLightFactor;
    
    PSOutput_MRT pout;
    pout.Position = float4(pin.PosW, 0.f);
    pout.Normal = float4(bumpedNormalW, 0.f);
    pout.Diffuse = diffuse;
    pout.Emissive = emissiveMapSample + gRimLightColor * rim;
    pout.MetallicSmoothness = float2(metallic, roughness);
    
    return pout;
}
