#include "Light.hlsl"

struct VSOutput_Lighting {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

struct PSOutput_Lighting {
    float4 Diffuse  : SV_TARGET0;
    float4 Specular : SV_TARGET1;
};

PSOutput_Lighting PSDirLighting(VSOutput_Lighting pin)
{
    PSOutput_Lighting pout;
    
    float3 posW = gTextureMap[gPassCB.RT0_PositionIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;

    // 카메라 기준 z값이 뒤에 있다면 그리지 않는다.
    float3 posV = mul(float4(posW, 1.f), gPassCB.MtxView);
    if (posV.z <= 0.f)
        clip(-1);
    
    float3 normalW = gTextureMap[gPassCB.RT1_NormalIndex].Sample(gsamAnisotropicWrap, pin.UV).xyz;
    float4 diffuseAlbedo = gTextureMap[gPassCB.RT2_DiffuseIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float2 metallicSmoothness = gTextureMap[gPassCB.RT3_MetallicSmoothnessIndex].Sample(gsamAnisotropicWrap, pin.UV).xy;
    
    float3 toCameraW = gPassCB.CameraPos - posW;
    
    float3 shadowFactor = 1.f;
    Material mat = { diffuseAlbedo, metallicSmoothness.x, metallicSmoothness.y };
    LightColor lightColor = ComputeLighting(mat, posW, normalW, toCameraW, shadowFactor);
    
    pout.Diffuse = float4(lightColor.Diffuse, 0.f);
    pout.Specular = float4(lightColor.Specular, 0.f);
    
    return pout;
}
