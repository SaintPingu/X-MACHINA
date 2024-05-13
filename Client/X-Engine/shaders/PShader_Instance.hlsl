#include "Common.hlsl"

struct VSOutput_Inst {
    float4 PosH  : SV_POSITION;
    float3 PosW  : POSITIONW;
    float4 Color : COLOR;
};

struct PSOutput_MRT {
    float4 Texture : SV_TARGET0;
    //float  Distance : SV_TARGET4;
};

PSOutput_MRT PSInstancing(VSOutput_Inst input)
{
    PSOutput_MRT output;
    if (gObjectCB.MatIndex == 0)
    {
        output.Texture = float4(1, 1, 1, 1);
    }
    else
    {
        MaterialInfo mat = gMaterialBuffer[gObjectCB.MatIndex];
    
        output.Texture = mat.Diffuse;
    }
    
    return output;
}