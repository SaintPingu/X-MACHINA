#define MaterialInfo_Albedo			0x01
#define MaterialInfo_Specular		0x02
#define MaterialInfo_Normal			0x04
#define MaterialInfo_Metalic    	0x08
#define MaterialInfo_Emission		0x10
#define MaterialInfo_DetailAlbedo	0x20
#define MaterialInfo_DetailNormal	0x40
#define MaterialInfo_Sprite         0x80

struct MaterialInfo {
    float4 Ambient;
    float4 Diffuse;
    float4 Sepcular; //a = power
    float4 Emissive;
};

cbuffer cbGameObject : register(b5) {
    matrix gWorld : packoffset(c0);
};

cbuffer cbGameObjectInfo : register(b0) {
    matrix gMtxWorld : packoffset(c0);
    MaterialInfo gMaterial : packoffset(c4);
    uint gTextureMask : packoffset(c8);
};

cbuffer cbCameraInfo : register(b1) {
    matrix gMtxView : packoffset(c0);
    matrix gMtxProj : packoffset(c4);
    float3 gCameraPos : packoffset(c8);
};

cbuffer cbSpriteInfo : register(b4) {
    matrix gMtxSprite : packoffset(c0);
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

