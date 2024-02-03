#include "Light.hlsl"

struct VSOutput_Water
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 UV      : UV;
};

float4 PSWater(VSOutput_Water pin) : SV_TARGET
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuseAlbedo = matInfo.Diffuse;
    float roughness      = 0.1f;
    float metallic       = 0.1f;
    int diffuseMapIndex  = matInfo.DiffuseMap0Index;
    int normalMapIndex   = matInfo.NormalMapIndex;
    
    if (diffuseMapIndex != -1)
    {
        // diffuseMap을 사용할 경우 샘플링하여 계산한다.
        diffuseAlbedo *= GammaDecoding(gTextureMap[diffuseMapIndex].Sample(gSamplerState, pin.UV));
    }
    
    pin.NormalW = normalize(pin.NormalW);
    
    // 해당 픽셀에서 카메라까지의 벡터
    float3 toCameraW = normalize(gPassCB.CameraPos - pin.PosW);
    
    // 전역 조명의 ambient 값을 계산한다.
    float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;
    
    float3 shadowFactor = 1.f;
    Material mat = { diffuseAlbedo, metallic, roughness };
    LightColor lightColor = ComputeLighting(mat, pin.PosW, pin.NormalW, toCameraW, shadowFactor);
    
    float4 litColor = ambient + float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    
    // 감마 디코딩을 수행했을 경우에만 감마 인코딩을 한다.
    if(diffuseMapIndex != -1)
    {
        litColor = GammaEncoding(litColor);
    }
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}