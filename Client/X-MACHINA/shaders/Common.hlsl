#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT
#define POST_PROCESSING

#define LightType_Spot          0
#define LightType_Directional   1
#define LightType_Point         2
#define gkMaxTexture            200
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
    float3  Padding;
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
    float3  Padding;
};

struct PassInfo {
    matrix      MtxView;
    matrix      MtxProj;
    float3      CameraPos;
    float       DeltaTime;

    LightInfo   Lights[gkMaxSceneLight];
    uint        LightCount;
    float3      Padding;
    
    float4      GlobalAmbient;
    float4      FogColor;
    
    float       FogStart;
    float       FogRange;
    int         RT0_PositionIndex;
    int         RT1_NormalIndex;
    
    int         RT2_DiffuseIndex;
    int         RT3_EmissiveIndex;
    int         RT4_MetallicSmoothnessIndex;
    int         FilterOption;
    
    int         RT0L_DiffuseIndex;
    int         RT1L_SpecularIndex;
    int         RT2L_AmbientIndex;
    float       Padding2;
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

ConstantBuffer<ObjectInfo> gObjectCB     : register(b0);
ConstantBuffer<PassInfo> gPassCB         : register(b1);
ConstantBuffer<PostPassInfo> gPostPassCB : register(b2);
ConstantBuffer<ColliderInfo> gColliderCB : register(b3);
ConstantBuffer<BoneTransformInfo> gSkinMeshCB : register(b4);

StructuredBuffer<SB_StandardInst> gInstBuffer   : register(t0);
StructuredBuffer<SB_ColorInst> gColorInstBuffer : register(t0);
StructuredBuffer<MaterialInfo> gMaterialBuffer  : register(t0, space1);

TextureCube  gSkyBoxTexture            : register(t1);
Texture2D    gTextureMap[gkMaxTexture] : register(t2); // t2, t3, t4...

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

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

#endif