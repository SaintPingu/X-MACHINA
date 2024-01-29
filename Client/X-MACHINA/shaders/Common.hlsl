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
    
    int         RT1_TextureIndex;
    int         RT2_UIIndex;
    int         RT3_NormalIndex;
    int         RT4_DistanceIndex;
    int         RT5_DepthIndex;
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
ConstantBuffer<ColliderInfo> gColliderCB : register(b2);

StructuredBuffer<SB_StandardInst> gInstBuffer   : register(t0);
StructuredBuffer<SB_ColorInst> gColorInstBuffer : register(t0);
StructuredBuffer<MaterialInfo> gMaterialBuffer  : register(t0, space1);

TextureCube     gSkyBoxTexture            : register(t1);
Texture2D       gTextureMap[gkMaxTexture] : register(t2); // t2, t3, t4...
SamplerState    gSamplerState             : register(s0);

#endif