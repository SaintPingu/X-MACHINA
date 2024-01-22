#include "Common.hlsl"

cbuffer cbGameObjectInfo : register(b0) {
    matrix          gMtxWorld : packoffset(c0);
    MaterialInfo    gMaterial : packoffset(c4);
    uint            gTextureMask : packoffset(c8);
};

cbuffer cbObject : register(b1) {
    matrix      gWorld;
    matrix      gMtxSprite;
    int         gMatSBIdx;
    float3      gPadding;
};

cbuffer cbPass : register(b2) {
    matrix          gMtxView;
    matrix          gMtxProj;
    float3          gCameraPos;
    float           gDeltaTime;

    LightInfo       gLights[gkMaxSceneLight];
    float4          gGlobalAmbient;
    
    float4          gFogColor;
    float           gFogStart;
    float           gFogRange;
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

StructuredBuffer<TestMaterial> materialBuffer : register(t4, space1);
