#include "Light.hlsl"

struct PSOutput_MRT {
    float4 Texture  : SV_TARGET1;
    float  Distance : SV_TARGET4;
};

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

#ifdef POST_PROCESSING
PSOutput_MRT PSTexturedLightingToMultipleRTs(VSOutput_Standard input)
{
    PSOutput_MRT output;
    MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    
    input.NormalW = normalize(input.NormalW);
    
    float4 illumination = Lighting(mat, input.PosW, input.NormalW);
    
    //output.Position = float4(input.PositionW, 1.f);
    //output.normal = float4(input.NormalW.xyz * 0.5f + 0.5f, 1.f);
    output.Distance = length(input.PosW - gPassCB.CameraPos);
    
    if (mat.DiffuseMap0Index != -1)
        output.Texture = gTextureMap[mat.DiffuseMap0Index].Sample(gSamplerState, input.UV) * illumination;
    else 
        output.Texture = illumination;
    
    return output;
}
#else
float4 PSTexturedLightingToMultipleRTs(VSOutput_Standard input) : SV_TARGET
{
    return gAlbedoTexture.Sample(gSamplerState, input.UV);
}

#endif