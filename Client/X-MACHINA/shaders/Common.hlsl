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

struct LightInfo {
    float4  Ambient;
    float4  Diffuse;
    float4  Sepcular;
    
    float3  Position;
    float   Falloff;
    
    float3  Direction;
    float   Theta; //cos(Theta)
    
    float3  Attenuation;
    float   Phi; //cos(Phi)
    
    float   Range;
    float   Padding;
    int     Type;
    bool    Enable;
};

struct MaterialInfo {
    float4  Ambient;
    float4  Diffuse;
    float4  Sepcular;
    float4  Emissive;
    
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
    float4      GlobalAmbient;
    
    float4      FogColor;
    
    float       FogStart;
    float       FogRange;
    
    int         RT0_TextureIndex;
    int         RT1_UIIndex;
    int         RT2_NormalIndex;
    int         RT3_DepthIndex;
    int         RT4_DistanceIndex;
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





ConstantBuffer<ObjectInfo> gObjectCB          : register(b0);
ConstantBuffer<PassInfo> gPassCB              : register(b1);
ConstantBuffer<BoneTransformInfo> gSkinMeshCB : register(b2);
ConstantBuffer<ColliderInfo> gColliderCB      : register(b3);

StructuredBuffer<SB_StandardInst> gInstBuffer   : register(t0);
StructuredBuffer<SB_ColorInst> gColorInstBuffer : register(t0);
StructuredBuffer<MaterialInfo> gMaterialBuffer  : register(t0, space1);

TextureCube     gSkyBoxTexture            : register(t1);
Texture2D       gTextureMap[gkMaxTexture] : register(t2); // t2, t3, t4...
SamplerState    gSamplerState             : register(s0);

// 디스플레이 출력은 어두운 부분을 더 자세히 표현하기 위해서 이미지를 Decoding하여 출력한다.
// 이로 인해, 대부분의 텍스처는 전체적으로 어두운 부분을 해결하기 위해 Encoding되어 저장된다.
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

float4 Mix(float4 colorA, float4 colorB, float t)
{
    return colorA * (1 - t) + colorB * t;
}

#endif