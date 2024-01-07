#include "Light.hlsl"

struct VSOutput_Terrain {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV0 : UVA;
    float2 UV1 : UVB;
};

struct PSOutput_MRT {
    float4 Texture : SV_TARGET1;
    float  Distance : SV_TARGET4;
};

#ifdef POST_PROCESSING
PSOutput_MRT PSTerrain(VSOutput_Terrain input)
{
    PSOutput_MRT output;
    
    float4 illumination = Lighting(input.PositionW, input.NormalW);
    
    float4 splatColor = gTerrainSplatMap.Sample(gSamplerState, input.UV1);
    float4 layer0 = float4(0, 0, 0, 0);
    float4 layer1 = float4(0, 0, 0, 0);
    float4 layer2 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.f)
    {
        layer0 = gTerrainTextureLayer0.Sample(gSamplerState, input.UV0);
        layer0 *= splatColor.r;

    }
    if (splatColor.g > 0.f)
    {
        //layer1 = gTerrainTextureLayer1.Sample(gSamplerState, input.UV0);
        //layer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        //layer2 = gTerrainTextureLayer2.Sample(gSamplerState, input.UV0);
        //layer2 *= splatColor.b;
    }

    float4 texColor = normalize(layer0 + layer1 + layer2);
    
    output.Texture  = lerp(illumination, texColor, .5f);
    output.Distance = length(input.PositionW - gCameraPos);
    
    return output;
}
#else
float4 PSTerrain(VSOutput_Terrain input) : SV_TARGET
{
    float4 illumination = Lighting(input.PositionW, input.NormalW);
    
    float4 splatColor = gTerrainSplatMap.Sample(gSamplerState, input.UV1);
    float4 layer0 = float4(0, 0, 0, 0);
    float4 layer1 = float4(0, 0, 0, 0);
    float4 layer2 = float4(0, 0, 0, 0);
    
    if (splatColor.r > 0.1f)
    {
        layer0 = gTerrainTextureLayer0.Sample(gSamplerState, input.UV0);
        layer0 *= splatColor.r;
    }
    if (splatColor.g > 0.f)
    {
        layer1 = gTerrainTextureLayer1.Sample(gSamplerState, input.UV0);
        layer1 *= splatColor.g;
    }
    if (splatColor.b > 0.f)
    {
        layer2 = gTerrainTextureLayer2.Sample(gSamplerState, input.UV0);
        layer2 *= splatColor.b;
    }
    if (splatColor.a > 0.f)
    {
        layer3 = gTerrainTextureLayer3.Sample(gSamplerState, input.UV0);
        layer3 *= splatColor.a;
    }
    float4 texColor = normalize(layer0 + layer1 + layer2);
    
    return saturate(illumination * 0.5 + texColor * 0.5f);;
}
#endif