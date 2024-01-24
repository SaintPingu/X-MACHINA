#include "Common.hlsl"

struct PSOutput_MRT {
    float4 Texture : SV_TARGET1;
    float Distance : SV_TARGET4;
};

struct VSOutput_TexInst {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITIONW;
    float2 UV : UV;
};

#ifdef POST_PROCESSING
PSOutput_MRT PSTextureInstancing(VSOutput_TexInst input)
{
    MaterialInfo mat = materialBuffer[gMatIndex];
    
    PSOutput_MRT output;
    output.Texture = gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV);
    output.Distance = length(input.PositionW - gCameraPos);
    
    return output;
}
#else
float4 PSTextureInstancing(VSOutput_TexInst input) : SV_TARGET
{
    return gAlbedoTexture.Sample(gSamplerState, input.UV);
}
#endif