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
    float4 Texture  : SV_TARGET0;
    float  Distance : SV_TARGET4;
};

PSOutput_MRT PSDeferred(VSOutput_Standard pin)
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuseAlbedo = matInfo.Diffuse;
    float metallic       = matInfo.Metallic;
    float roughness      = matInfo.Roughness;
    int diffuseMapIndex  = matInfo.DiffuseMap0Index;
    int normalMapIndex   = matInfo.NormalMapIndex;
    int roughMapIndex    = matInfo.RoughnessMapIndex;
    
    // diffuseMap을 사용할 경우 샘플링하여 계산
    if (diffuseMapIndex != -1)
    {
        diffuseAlbedo *= gTextureMap[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    }

    // LUT 필터 옵션이 켜진 상태에서만 감마 디코딩을 수행
    if (gPassCB.FilterOption & Filter_Tone)
    {
        diffuseAlbedo = GammaDecoding(diffuseAlbedo);
    }
    
    pin.NormalW = normalize(pin.NormalW);
    float3 bumpedNormalW = pin.NormalW;
    float4 normalMapSample = (float4)0;
    
    // normal map을 사용할 경우 샘플링하여 월드 공간으로 변환
    if (normalMapIndex != -1)
    {
        normalMapSample = gTextureMap[normalMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }
    // roughness map을 사용할 경우 샘플링하여 roughness 값 계산
    if (roughMapIndex != -1)
    {
        roughness *= gTextureMap[roughMapIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    }
    
    // 해당 픽셀에서 카메라까지의 벡터
    float3 toCameraW = gPassCB.CameraPos - pin.PosW;
    
    // 전역 조명의 ambient 값을 계산
    float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;
    
    float3 shadowFactor = 1.f;
    Material mat = { diffuseAlbedo, metallic, roughness };
    LightColor lightColor = ComputeLighting(mat, pin.PosW, bumpedNormalW, toCameraW, shadowFactor);
    
    float4 litColor = ambient +float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    
    //// specular reflection
	//float3 r = reflect(-toCameraW, bumpedNormalW);
	//float4 reflectionColor = gSkyBoxTexture.Sample(gSamplerState, r);
	//float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
	//litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    litColor.a = diffuseAlbedo.a;
    
    PSOutput_MRT output;
    output.Texture = litColor;
    output.Distance = length(toCameraW);
    
    return output;
}
