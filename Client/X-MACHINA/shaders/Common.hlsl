#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT
#define POST_PROCESSING

#define LightType_Spot          0
#define LightType_Directional   1
#define LightType_Point         2
#define gkMaxTexture            100
#define gkMaxSceneLight         32

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
    float4  DiffuseAlbedo;
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
    int HeightMapIndex;
    int ShadowMapIndex;
    int RoughnessMapIndex;
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
    
    int         RT0_TextureIndex;
    int         RT1_UIIndex;
    int         RT2_NormalIndex;
    int         RT3_DepthIndex;
    int         RT4_DistanceIndex;
    
    int         RT0_OffScreenIndex;
    float2      Padding2;
};

struct PostPassInfo {
    int         RT0_OffScreenIndex;
    float3      Padding;
};

struct ColliderInfo {
    matrix MtxView;
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

// ���÷��� ����� ��ο� �κ��� �� �ڼ��� ǥ���ϱ� ���ؼ� �̹����� Decoding(��Ӱ�)�Ͽ� ����Ѵ�.
// �̷� ����, ��κ��� �ؽ�ó�� ��ü������ ��ο� �κ��� �ذ��ϱ� ���� Encoding(���)�Ǿ� ����ȴ�.
// ���� ��, ����� ���õ� ������ ���� ������ �ƴ� ���� �������� ó���ؾ� �Ѵ�.

// ���� ������ �����ϴ� �Լ�
float4 GammaEncoding(float4 color)
{
    return float4(pow(color.rgb, 1 / 2.2f), color.a);
}

// ���� ������ �����ϴ� �Լ�
float4 GammaDecoding(float4 color)
{
    return float4(pow(color.rgb, 2.2f), color.a);
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    // [0,1] �������� [-1,1]�� �����Ѵ�.
    float3 normalT = 2.0f * normalMapSample - 1.0;

    // orthonormal basis(TBN ���)�� �����Ѵ�.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // tangent �������� world �������� ��ȯ�Ѵ�.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

bool IsWhite(float4 color)
{
    return color.rgb == float3(1.f, 1.f, 1.f) && color.a == 1.f;
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