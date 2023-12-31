TextureCube gSkyBoxTexture : register(t2);

Texture2D gAlbedoTexture : register(t2);
Texture2D gTerrainTextureLayer0 : register(t2);
Texture2D gTerrainTextureLayer1 : register(t3);
Texture2D gTerrainTextureLayer2 : register(t4);
Texture2D gTerrainTextureLayer3 : register(t5);
Texture2D gTerrainSplatMap : register(t6);

Texture2D gRT1_Texture : register(t7);
Texture2D gRT2_UI : register(t8);
Texture2D gRT3_Normal : register(t9);
Texture2D gRT4_Distance : register(t10);
Texture2D gRT5_Depth : register(t11);

SamplerState gSamplerState : register(s0);

cbuffer cbGameInfo : register(b3)
{
    float gDeltaTime : packoffset(c0);
};

bool IsWhite(float4 color)
{
    return color.rgb == float3(1.f, 1.f, 1.f) && color.a == 1.f;
}

#define POST_PROCESSING