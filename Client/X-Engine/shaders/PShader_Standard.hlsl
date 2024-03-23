#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

float4 PSStandard(VSOutput_Standard pin) : SV_TARGET
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse       = matInfo.Diffuse;
    float metallic       = matInfo.Metallic;
    float roughness      = matInfo.Roughness;
    int diffuseMapIndex  = matInfo.DiffuseMap0Index;
    int normalMapIndex   = matInfo.NormalMapIndex;
    int metallicMapIndex = matInfo.MetallicMapIndex;
    int emissiveMapIndex = matInfo.EmissiveMapIndex;
    
    // diffuseMap�� ����� ��� ���ø��Ͽ� ���
    if (diffuseMapIndex != -1)
    {
        diffuse *= GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
    }
    
    pin.NormalW = normalize(pin.NormalW);
    float4 normalMapSample = (float4)0;
    
    // normal map�� ����� ��� ���ø��Ͽ� ���� �������� ��ȯ
    float3 bumpedNormalW = (float4)0;
    if (normalMapIndex != -1)
    {
        normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }
    
    // sampling emissiveMap
    float4 emissiveMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        emissiveMapSample = gTextureMaps[emissiveMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }
    
    // roughness map�� ����� ��� ���ø��Ͽ� roughness �� ���
    if (metallicMapIndex != -1)
    {
        metallic *= gTextureMaps[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    }
    
    // �ش� �ȼ����� ī�޶������ ����
    float3 toCameraW = gPassCB.CameraPos - pin.PosW;
    
    // ���� ������ ambient ���� ����Ѵ�.
    float4 ambient = gPassCB.GlobalAmbient * diffuse;
    
    // ��Ż�� ���� ����
    float3 diffuseAlbedo = lerp(diffuse.xyz, 0.0f, metallic);
    float3 specularAlbedo = lerp(0.03f, diffuse.xyz, metallic);
    Material mat = { diffuseAlbedo, specularAlbedo, metallic, roughness };
    
    float3 shadowFactor = 1.f;
    LightColor lightColor = ComputeLighting(mat, pin.PosW, bumpedNormalW, toCameraW, shadowFactor);
    
    //// specular reflection
    //float3 r = reflect(-toCameraW, bumpedNormalW);
    //float4 reflectionColor = gSkyBoxTexture[gPassCB.SkyBoxIndex].Sample(gsamLinearWrap, r);
    //float3 fresnelFactor = SchlickFresnel(specularAlbedo, bumpedNormalW, r);
    //float3 reflection = (metallic) * fresnelFactor * reflectionColor.rgb;
    
    //// rim light
    //float rimWidth = 0.7f;
    //float gRimLightFactor = 0.f;
    //float4 gRimLightColor = float4(1.f, 0.6f, 0.f, 0.f);
    //float rim = 1.0f - max(0, dot(bumpedNormalW, normalize(gPassCB.CameraPos - pin.PosW)));
    //rim = smoothstep(1.0f - rimWidth, 1.0f, rim) * gRimLightFactor;
    
    float4 litColor = ambient/* + gRimLightColor * rim + float4(reflection, 0.f)*/ + float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    litColor.a = diffuse.a;
    
    return litColor;
}