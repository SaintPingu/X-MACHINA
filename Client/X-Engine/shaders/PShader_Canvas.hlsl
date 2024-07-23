#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    float4 color;
    // ��Ƽ������ ������� �ʴ� ��� MaterialIndex�� �ٷ� �ؽ�ó �ε����� Set�� ��
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
