#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT
#define POST_PROCESSING

#define LightType_Spot          0
#define LightType_Directional   1
#define LightType_Point         2
#define gkMaxTextureCount       1024
#define gkMaxSkyBoxCount        16
#define gkMaxSceneLight         32

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

#define Filter_None           0x01
#define Filter_Blur           0x02
#define Filter_Tone           0x04
#define Filter_LUT            0x08

struct LightInfo
{
    float3  Strength;
    float   FalloffStart;   // point/spot light only
    float3  Direction;      // directional/spot light only
    float   FalloffEnd;     // point/spot light only
    float3  Position;       // point light only
    float   SpotPower;      // spot light only
    int     LightType;
    int     IsEnable;
	int		ObjCBIndex;
    int     Padding;
};

struct LightColor
{
    float3  Diffuse;
    float3  Specular;
};

struct Material
{
    float3  DiffuseAlbedo;
    float3  SpecularAlbedo;
    float   Metallic;
    float   Roughness;
};

struct MaterialInfo {
    float4  Diffuse;
    float   Metallic;
    float   Roughness;
    float2  Padding;
    
    int DiffuseMap0Index;
    int DiffuseMap1Index;
    int DiffuseMap2Index;
    int DiffuseMap3Index;
    
    int NormalMapIndex;
    int EmissiveMapIndex;
    int MetallicMapIndex;
    int OcclusionMapIndex;
};

struct ObjectInfo {
    matrix  MtxWorld;
    matrix  MtxSprite;
    int     MatIndex;
    int     LightIndex;
    float2  Padding;
};

struct PassInfo {
    matrix      MtxView;
    matrix      MtxProj;
    matrix      MtxShadow;
    float3      CameraPos;
    uint        LightCount;
    LightInfo   Lights[gkMaxSceneLight];
    
    float       DeltaTime;
    float       TotalTime;
    int         FrameBufferWidth;
    int         FrameBufferHeight;
    
    float4      GlobalAmbient;
    float4      FogColor;
    
    float       FogStart;
    float       FogRange;
    int         SkyBoxIndex;
    int         ShadowIndex;
    
    int         FilterOption;
    float       ShadowIntensity;
    float2      Padding;
    
    int         RT0_PositionIndex;
    int         RT1_NormalIndex;
    int         RT2_DiffuseIndex;
    int         RT3_EmissiveIndex;
    
    int         RT4_MetallicSmoothnessIndex;
    int         RT5_OcclusionIndex;
    int         RT0L_DiffuseIndex;
    int         RT1L_SpecularIndex;
    
    int         RT2L_AmbientIndex;
};

struct PostPassInfo {
    int         RT0_OffScreenIndex;
    float3      Padding;
};

struct ColliderInfo {
    matrix MtxView;
};

struct BoneTransformInfo {
    float4x4 BoneTransforms[SKINNED_ANIMATION_BONES];
};

struct SB_StandardInst {
    matrix MtxObject;
};

struct SB_ColorInst {
    matrix MtxObject;
    float4 Color;
};

ConstantBuffer<ObjectInfo> gObjectCB            : register(b0);
ConstantBuffer<PassInfo> gPassCB                : register(b1);
ConstantBuffer<PostPassInfo> gPostPassCB        : register(b2);
ConstantBuffer<ColliderInfo> gColliderCB        : register(b3);
ConstantBuffer<BoneTransformInfo> gSkinMeshCB   : register(b4);

StructuredBuffer<SB_StandardInst> gInstBuffer   : register(t0);
StructuredBuffer<SB_ColorInst> gColorInstBuffer : register(t0);
StructuredBuffer<MaterialInfo> gMaterialBuffer  : register(t0, space1);

TextureCube gSkyBoxMaps[gkMaxSkyBoxCount]       : register(t1, space1);
Texture2D gTextureMaps[gkMaxTextureCount]       : register(t1); // t1, t2, t3...

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// 디스플레이 출력은 어두운 부분을 더 자세히 표현하기 위해서 이미지를 Decoding(어둡게)하여 출력한다.
// 이로 인해, 대부분의 텍스처는 전체적으로 어두운 부분을 해결하기 위해 Encoding(밝게)되어 저장된다.
// 따라서 빛, 조명과 관련된 연산은 비선형 공간이 아닌 선형 공간에서 처리해야 한다.

// 감마 보정을 적용하는 함수
float4 GammaEncoding(float4 color)
{
    return float4(pow(color.rgb, 1 / 2.2f), color.a);
}

// 감마 보정을 해제하는 함수
float4 GammaDecoding(float4 color)
{
    return float4(pow(color.rgb, 2.2f), color.a);
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    // [0,1] 공간에서 [-1,1]로 매핑한다.
    float3 normalT = 2.0f * normalMapSample - 1.0;

    // orthonormal basis(TBN 행렬)를 생성한다.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // tangent 공간에서 world 공간으로 변환한다.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float Mix(float x, float y, float alpha)
{
    return x * (1.f - alpha) + y * x;
}

float4 Fog(float4 color, float3 position)
{
    float3 camPos = gPassCB.CameraPos;
    float3 posToCam = camPos - position;
    float distance = length(posToCam);
    
    float factor = min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}

float4 FogDistance(float4 color, float3 distance)
{
    float factor = min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}

bool IsOutOfRange(float3 value, float lhs = 0.f, float rhs = 1.f)
{
    return (value.x < 0.f || value.x > 1.f ||
            value.y < 0.f || value.y > 1.f ||
            value.z < 0.f || value.z > 1.f);
}

float ComputeShadowFactor(float4 shadowPosH)
{
    // w로 나눠서 투영을 완료한다.
    shadowPosH.xyz /= shadowPosH.w;
    
    // 절두체 범위의 밖에 있다면 계산하지 않고 바로 리턴한다.
    if (IsOutOfRange(shadowPosH.xyz))
        return 1.f;
    
    // z값이 깊이 값이다.
    float depth = shadowPosH.z;
    
    uint width, height, numMips;
    gTextureMaps[gPassCB.ShadowIndex].GetDimensions(0, width, height, numMips);
    
    // 텍셀 사이즈
    float dx = 1.f / (float)width;
    
    // 비율 근접 필터링(PCF)
    float percentLit = 0.f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };
    
    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += gTextureMaps[gPassCB.ShadowIndex].SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.f;
}

#endif