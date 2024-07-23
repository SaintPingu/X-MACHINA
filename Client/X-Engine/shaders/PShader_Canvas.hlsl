#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    float4 color;
    // 머티리얼을 사용하지 않는 경우 MaterialIndex에 바로 텍스처 인덱스를 Set할 것
    if (gObjectCB.UseOutline)
    {
        color = float4(gObjectCB.HitRimColor, 1.f);
    }
    else
    {
        color = gTextureMaps[gObjectCB.MatIndex].Sample(gsamLinearWrap, input.UV);
    }
    color.a *= gObjectCB.AlphaIntensity;
    
    if (input.UV.x > gObjectCB.SliderValue)
    {
        discard;
    }

    return color;
}
