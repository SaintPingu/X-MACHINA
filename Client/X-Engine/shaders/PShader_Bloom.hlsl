#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSBloom(VSOutput_Tex pin) : SV_TARGET
{
    float4 offScreen = gTextureMaps[gPassCB.RT0O_OffScreenIndex].Sample(gsamLinearClamp, pin.UV);
    float4 bloom = gTextureMaps[gPassCB.BloomIndex].Sample(gsamLinearClamp, pin.UV);
    
    return offScreen + bloom;
}
