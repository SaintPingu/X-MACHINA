#include "Common.hlsl"

struct VSOutput_Inst {
    float4 PosH  : SV_POSITION;
    float3 PosW  : POSITIONW;
    float4 Color : COLOR;
};


/*
// WITH_POSTPROCESSING - has error (color isn't apply)

struct PSOutput_MRT {
    float4 Texture : SV_TARGET1;
    float  Distance : SV_TARGET4;
};

PSOutput_MRT PSInstancing(VSOutput_Inst input)
{
    PSOutput_MRT output;
    output.Texture = input.Color;
    output.Distance = input.PositionW;
    
    return output;
}
*/

float4 PSInstancing(VSOutput_Inst input) : SV_TARGET
{
    return input.Color;
}