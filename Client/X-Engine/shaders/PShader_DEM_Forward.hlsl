#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

float4 PSDEMForward(VSOutput_Standard pin) : SV_TARGET0
{
    // material info
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse        = matInfo.Diffuse;
    float metallic        = matInfo.Metallic;
    float roughness       = matInfo.Roughness;
    float occlusion       = 1.f;
    int diffuseMapIndex   = matInfo.DiffuseMap0Index;
    int normalMapIndex    = matInfo.NormalMapIndex;
    int metallicMapIndex  = matInfo.MetallicMapIndex;
    int emissiveMapIndex  = matInfo.EmissiveMapIndex;
    int occlusionMapIndex = matInfo.OcclusionMapIndex;
    
    // sampling diffuseMap
    if (diffuseMapIndex != -1)
    {
        diffuse *= GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
    }

    // normalize normal
    pin.NormalW = normalize(pin.NormalW);
    float3 bumpedNormalW = pin.NormalW;
    
    // sampling normalMap, to world space
    float4 normalMapSample = (float4)0;
    if (normalMapIndex != -1)
    {
        normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }
    
    // sampling emissiveMap
    float4 emissive = (float4)0;
    if (metallicMapIndex != -1)
    {
        emissive = gTextureMaps[emissiveMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }
    
    // sampling metallicMap
    float4 metallicMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        metallicMapSample = GammaDecoding(gTextureMaps[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        metallic = metallicMapSample.r;
        roughness = 1 - metallicMapSample.a;
    }
    
    float3 toCameraW = normalize(gPassCB.CameraPos - pin.PosW);
    
    // apply rim light
    float4 hitRimLight = ComputeRimLight(float4(gObjectCB.HitRimColor, 1.f), 0.6f, gObjectCB.HitRimFactor, pin.PosW, bumpedNormalW);
    float4 mindRimLight = ComputeRimLight(float4(gObjectCB.MindRimColor, 1.f),0.6f, gObjectCB.MindRimFactor, pin.PosW, bumpedNormalW);
    
    // 메탈릭 값을 적용
    float3 diffuseAlbedo = lerp(diffuse.xyz, 0.0f, metallic);
    float3 specularAlbedo = lerp(0.03f, diffuse.xyz, metallic);
    Material mat = { diffuseAlbedo, specularAlbedo, metallic, roughness };
    
    // 조명 계산
    LightColor lightColor = ComputeDirectionalLight(gPassCB.Lights[gObjectCB.LightIndex], mat, pin.PosW, bumpedNormalW, toCameraW, 1.f);
    
    //// specular reflection
    //float3 r = reflect(-toCameraW, bumpedNormalW);
    //float4 reflectionColor = gSkyBoxMaps[gPassCB.SkyBoxIndex].Sample(gsamLinearWrap, r);
    //float3 fresnelFactor = SchlickFresnel(specularAlbedo, bumpedNormalW, r);
    //float3 reflection = (metallic) * fresnelFactor * reflectionColor.rgb;
    
    // litColor
    float4 litDiffuse = GammaEncoding(float4(lightColor.Diffuse, 1.f));
    float4 litSpecular = GammaEncoding(float4(lightColor.Specular, 1.f)) /*+ float4(reflection, 1.f)*/;
    float4 litAmbient = GammaEncoding(diffuse * gPassCB.GlobalAmbient) + emissive;
    
    float4 litColor = litAmbient + litDiffuse + litSpecular + hitRimLight + mindRimLight;
    
    // temp
    float3 dissolveColor = float3(5.f, 1.f, 0.f);
    float4 dissolve = Dissolve(dissolveColor, gTextureMaps[gPassCB.LiveObjectDissolveIndex].Sample(gsamAnisotropicWrap, pin.UV * 2.f).x, gObjectCB.DeathElapsed);
    
    litColor.a = dissolve.a;
    litColor.rgb += dissolve.rgb;
    
    return litColor;
}