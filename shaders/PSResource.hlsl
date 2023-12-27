Texture2D albedoTexture : register(t2);
TextureCube skyBoxTexture : register(t2);

Texture2D terrainTextureLayer0 : register(t2);
Texture2D terrainTextureLayer1 : register(t3);
Texture2D terrainTextureLayer2 : register(t4);
Texture2D terrainTextureLayer3 : register(t5);
Texture2D terrainSplatMap : register(t6);

Texture2D rt1_texture : register(t7);
Texture2D rt2_ui : register(t8);
Texture2D rt3_normal : register(t9);
Texture2D rt4_distance : register(t10);
Texture2D depthTexture : register(t11);

SamplerState samplerState : register(s0);

cbuffer gameInfo : register(b3)
{
    float deltaTime : packoffset(c0);
};

bool IsWhite(float4 color)
{
    return color.rgb == float3(1.0f, 1.0f, 1.0f) && color.a == 1.0f;
}

#define POST_PROCESSING