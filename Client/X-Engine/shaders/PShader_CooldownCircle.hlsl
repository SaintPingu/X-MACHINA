#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCooldownCircle(VSOutput_Tex input) : SV_TARGET
{
    float4 color;
    color = gTextureMaps[gObjectCB.MatIndex].Sample(gsamLinearWrap, input.UV);

    float2 center = input.UV - float2(0.5, 0.5);
    
    float angle = atan2(center.y, center.x);
    
    if (angle < 0.f) {
        angle += 2.f * 3.141592f;
    }
    
    float normalizedAngle = angle / (2.f * 3.141592f);
    normalizedAngle = frac(0.75f - normalizedAngle);
    
    if (normalizedAngle > gObjectCB.SliderValueX) {
        discard;
    }
    
    return color;
}
