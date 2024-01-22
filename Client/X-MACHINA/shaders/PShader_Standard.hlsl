//#include "Light.hlsl"
#include "TempLight.hlsl"

struct VSOutput_Standard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
    bool   IsTexture : ISTEXTURE;
};

float4 PS_Standard(VSOutput_Standard input) : SV_TARGET
{
    //if (input.IsTexture) {
    //    float4 color = gAlbedoTexture.Sample(gSamplerState, input.UV);
    
    //    float3 normalW = normalize(input.NormalW);
    //    float4 illumination = Lighting(materialBuffer[gMatSBIdx], input.PositionW, normalW);
        
    //    return Fog(lerp(color, illumination, 0.5f), input.PositionW);
    //}
    //else {
        float3 normalW = normalize(input.NormalW);
        float4 color = Lighting(materialBuffer[gMatSBIdx], input.PositionW, normalW);
        return color;
    //}
}