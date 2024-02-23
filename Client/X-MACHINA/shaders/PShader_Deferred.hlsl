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
    float2 MetallicSmoothness : SV_TARGET3;
};

PSOutput_MRT PSDeferred(VSOutput_Standard pin)
{
    // material info
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuseAlbedo = matInfo.Diffuse;
    float metallic       = matInfo.Metallic;
    float roughness      = matInfo.Roughness;
    int diffuseMapIndex  = matInfo.DiffuseMap0Index;
    int normalMapIndex   = matInfo.NormalMapIndex;
    int metallicMapIndex = matInfo.MetallicMapIndex;
    
    // sampling diffuseMap
    if (diffuseMapIndex != -1)
    {
        diffuseAlbedo *= GammaDecoding(gTextureMap[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
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
    
    // sampling metallicMap
    float4 metallicMapSample = (float4)0;
    if (metallicMapIndex != -1)
    {
        metallicMapSample = GammaDecoding(gTextureMap[metallicMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        metallic = metallicMapSample.r;
        roughness = 1 - metallicMapSample.a;
    }
    
    // roughness map을 사용할 경우 샘플링하여 roughness 값 계산
    //if (roughMapIndex != -1)
    //{
    //    roughness *= gTextureMap[roughMapIndex].Sample(gsamAnisotropicWrap, pin.UV).x;
    //}
    
    // 해당 픽셀에서 카메라까지의 벡터
    //float3 toCameraW = gPassCB.CameraPos - pin.PosW;
    
    // 전역 조명의 ambient 값을 계산
    //float4 ambient = gPassCB.GlobalAmbient * diffuseAlbedo;
    
    //float3 shadowFactor = 1.f;
    //Material mat = { diffuseAlbedo, metallic, roughness };
    //LightColor lightColor = ComputeLighting(mat, pin.PosW, bumpedNormalW, toCameraW, shadowFactor);
    
    //float4 litColor = ambient +float4(lightColor.Diffuse, 0.f) + float4(lightColor.Specular, 0.f);
    
    //// specular reflection
	//float3 r = reflect(-toCameraW, bumpedNormalW);
	//float4 reflectionColor = gSkyBoxTexture.Sample(gSamplerState, r);
	//float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
	//litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    //litColor.a = diffuseAlbedo.a;
    
    PSOutput_MRT pout;
    pout.Position = float4(pin.PosW, 0.f);
    pout.Normal = float4(bumpedNormalW, 0.f);
    pout.Diffuse = diffuseAlbedo;
    pout.MetallicSmoothness = float2(metallic, roughness);
    //output.Distance = length(toCameraW);
    
    return pout;
}
