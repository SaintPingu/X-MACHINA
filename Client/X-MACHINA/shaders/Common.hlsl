#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define MaterialInfo_Albedo			0x01
#define MaterialInfo_Specular		0x02
#define MaterialInfo_Normal			0x04
#define MaterialInfo_Metalic    	0x08
#define MaterialInfo_Emission		0x10
#define MaterialInfo_DetailAlbedo	0x20
#define MaterialInfo_DetailNormal	0x40
#define MaterialInfo_Sprite         0x80

#define gkMaxSceneLight         32

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT

#define LightType_Spot          0
#define LightType_Directional   1
#define LightType_Point         2

#define POST_PROCESSING

struct LightInfo {
    float4 Ambient;
    float4 Diffuse;
    float4 Sepcular;
    
    float3 Position;
    float Falloff;
    
    float3 Direction;
    float Theta; //cos(Theta)
    
    float3 Attenuation;
    float Phi; //cos(Phi)
    
    float Range;
    float Padding;
    int Type;
    bool Enable;
};

struct MaterialInfo {
    float4  Ambient;
    float4  Diffuse;
    float4  Sepcular; //a = power
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

cbuffer cbObject : register(b0) {
    matrix      gMtxWorld;
    matrix      gMtxSprite;
    int         gMatIndex;
    float3      gcbObjectPadding;
};

cbuffer cbPass : register(b1) {
    matrix      gMtxView;
    matrix      gMtxProj;
    float3      gCameraPos;
    float       gDeltaTime;

    LightInfo   gLights[gkMaxSceneLight];
    float4      gGlobalAmbient;
    
    float4      gFogColor;
    float       gFogStart;
    float       gFogRange;
    
    int         gRT1_TextureIndex;
    int         gRT2_UIIndex;
    int         gRT3_NormalIndex;
    int         gRT4_DistanceIndex;
    int         gRT5_DepthIndex;
    float3      gcbPassPadding;
};

cbuffer cbCollider : register(b2) {
    matrix  gMtxWorldCollider;
};

struct SB_StandardInst {
    matrix MtxObject;
};
StructuredBuffer<SB_StandardInst> instBuffer : register(t0);

struct SB_ColorInst {
    matrix MtxObject;
    float4 Color;
};
StructuredBuffer<SB_ColorInst> colorInstBuffer : register(t0);

StructuredBuffer<MaterialInfo> materialBuffer : register(t0, space1);

TextureCube gSkyBoxTexture : register(t1);

Texture2D gTextureMap[100] : register(t2); // t2, t3, t4...

SamplerState gSamplerState : register(s0);

#endif