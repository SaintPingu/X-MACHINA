#include "Light.hlsl"

struct VSOutput_Lighting {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

struct PSOutput_Lighting {
    float4 Diffuse  : SV_TARGET0;
    float4 Specular : SV_TARGET1;
    float4 Ambient  : SV_TARGET2;
};

PSOutput_Lighting PSDirLighting(VSOutput_Lighting pin)
{
    PSOutput_Lighting pout;
    
    float3 posW = gTextureMaps[gPassCB.RT0G_PositionIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;

    // 카메라 기준 z값이 뒤에 있다면 그리지 않는다.
    float3 posV = mul(float4(posW, 1.f), gPassCB.MtxView);
    if (posV.z <= 0.f)
        clip(-1);
    
    float3 toCameraW = normalize(gPassCB.CameraPos - posW);
    
    float3 normalW = gTextureMaps[gPassCB.RT1G_NormalIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;
    float4 diffuse = gTextureMaps[gPassCB.RT2G_DiffuseIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float2 metallicSmoothness = gTextureMaps[gPassCB.RT4G_MetallicSmoothnessIndex].Sample(gsamAnisotropicWrap, pin.UV).xy;
    float1 occlusion = gTextureMaps[gPassCB.RT5G_OcclusionIndex].Sample(gsamAnisotropicWrap, pin.UV).r;
    float1 ambientAcess = 1.f;
    
    if (gPassCB.FilterOption & Filter_Ssao)
        ambientAcess = gTextureMaps[gPassCB.RT0S_SsaoIndex].Sample(gsamAnisotropicWrap, pin.UV).r * occlusion;
    
    // 메탈릭 값을 적용
    float3 diffuseAlbedo = lerp(diffuse.xyz, 0.0f, metallicSmoothness.x);
    float3 specularAlbedo = lerp(0.03f, diffuse.xyz, metallicSmoothness.x);
    Material mat = { diffuseAlbedo, specularAlbedo, metallicSmoothness.x, metallicSmoothness.y };
    
    // 조명 계산
    float4 shadowPosH = mul(float4(posW, 1.f), gPassCB.MtxShadow);
    float shadowFactor = clamp(ComputeShadowFactor(shadowPosH), gPassCB.ShadowIntensity, 1.f);
    LightColor lightColor = ComputeDirectionalLight(gPassCB.Lights[gObjectCB.LightIndex], mat, posW, normalW, toCameraW, shadowFactor);
    
    // specular reflection
    float3 r = reflect(-toCameraW, normalW);
    float4 reflectionColor = gSkyBoxMaps[gPassCB.SkyBoxIndex].Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(specularAlbedo, normalW, r);
    float3 reflection = (metallicSmoothness.r) * fresnelFactor * reflectionColor.rgb;
    
    pout.Diffuse = GammaEncoding(float4(lightColor.Diffuse, 0.f));
    pout.Specular = GammaEncoding(float4(lightColor.Specular, 0.f)) + float4(reflection, 0.f);
    pout.Ambient = GammaEncoding(diffuse * gPassCB.GlobalAmbient * ambientAcess);
    
    return pout;
}
