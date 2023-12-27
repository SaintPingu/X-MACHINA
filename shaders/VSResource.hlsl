#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40
#define MATERIAL_SPRITE         	0x80

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbGameObjectInfo : register(b0)
{
    matrix gmtxWorld : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint texturesMask : packoffset(c8);
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    float3 gf3CameraPosition : packoffset(c8);
};

cbuffer cbSpriteInfo : register(b4)
{
    matrix gmtxSprite : packoffset(c0);
};

struct STANDARD_INSTANCED_GAMEOBJECT_INFO
{
    matrix m_mtxGameObject;
};
StructuredBuffer<STANDARD_INSTANCED_GAMEOBJECT_INFO> instanceBuffer : register(t0);

struct COLORED_INSTANCED_GAMEOBJECT_INFO
{
    matrix m_mtxGameObject;
    float4 color;
};
StructuredBuffer<COLORED_INSTANCED_GAMEOBJECT_INFO> colorInstanceBuffer : register(t0);