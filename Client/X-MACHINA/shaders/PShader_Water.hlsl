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
        // diffuseMap�� ����� ��� ���ø��Ͽ� ����Ѵ�.
        diffuseAlbedo *= GammaDecoding(gTextureMap[diffuseMapIndex].Sample(gSamplerState, pin.UV));
    }
    
    pin.NormalW = normalize(pin.NormalW);
    
    // �ش� �ȼ����� ī�޶������ ����
    float3 toCameraW = normalize(gPassCB.CameraPos - pin.PosW);
    
    // ���� ������ ambient ���� ����Ѵ�.
    float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;
    
    float3 shadowFactor = 1.f;
    Material mat = { diffuseAlbedo, metallic, roughness };
    LightColor lightColor = ComputeLighting(mat, pin.PosW, pin.NormalW, toCameraW, shadowFactor);
    
    float4 litColor = ambient + float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    
    // ���� ���ڵ��� �������� ��쿡�� ���� ���ڵ��� �Ѵ�.
    if(diffuseMapIndex != -1)
    {
        litColor = GammaEncoding(litColor);
    }
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}