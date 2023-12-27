#include "Light.hlsl"

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv0 : UVA;
    float2 uv1 : UVB;
};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cTexture : SV_TARGET1;
    float distance : SV_TARGET4;
};

#ifdef POST_PROCESSING
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTerrain(VS_TERRAIN_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    
    float4 illumination = Lighting(input.positionW, input.normalW);
    
    float4 splatColor = terrainSplatMap.Sample(samplerState, input.uv1);
    float4 cLayer0 = float4(0, 0, 0, 0);
    float4 cLayer1 = float4(0, 0, 0, 0);
    float4 cLayer2 = float4(0, 0, 0, 0);
    float4 cLayer3 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.1f)
    {
        cLayer0 = terrainTextureLayer0.Sample(samplerState, input.uv0);
        cLayer0 *= splatColor.r;
    }
    if (splatColor.g > 0.f)
    {
        cLayer1 = terrainTextureLayer1.Sample(samplerState, input.uv0);
        cLayer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        cLayer2 = terrainTextureLayer2.Sample(samplerState, input.uv0);
        cLayer2 *= splatColor.b;
    }
    if (splatColor.a > 0.f)
    {
        cLayer3 = terrainTextureLayer3.Sample(samplerState, input.uv0);
        cLayer3 *= splatColor.a;
    }
    float4 texColor = normalize(cLayer0 + cLayer1 + cLayer2 + cLayer3);
    
    output.cTexture = saturate(illumination * 0.5 + texColor * 0.5f);
    output.distance = length(input.positionW - gf3CameraPosition);
    
    return output;
}
#else
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float4 illumination = Lighting(input.positionW, input.normalW);
    
    float4 splatColor = terrainSplatMap.Sample(samplerState, input.uv1);
    float4 cLayer0 = float4(0, 0, 0, 0);
    float4 cLayer1 = float4(0, 0, 0, 0);
    float4 cLayer2 = float4(0, 0, 0, 0);
    float4 cLayer3 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.1f)
    {
        cLayer0 = terrainTextureLayer0.Sample(samplerState, input.uv0);
        cLayer0 *= splatColor.r;
    }
    if (splatColor.g > 0.f)
    {
        cLayer1 = terrainTextureLayer1.Sample(samplerState, input.uv0);
        cLayer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        cLayer2 = terrainTextureLayer2.Sample(samplerState, input.uv0);
        cLayer2 *= splatColor.b;
    }
    if (splatColor.a > 0.f)
    {
        cLayer3 = terrainTextureLayer3.Sample(samplerState, input.uv0);
        cLayer3 *= splatColor.a;
    }
    float4 texColor = normalize(cLayer0 + cLayer1 + cLayer2 + cLayer3);
    
    return saturate(illumination * 0.5 + texColor * 0.5f);;
}
#endif