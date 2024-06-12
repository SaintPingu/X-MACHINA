#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    // ��Ƽ������ ������� �ʴ� ��� MaterialIndex�� �ٷ� �ؽ�ó �ε����� Set�� ��
    float4 color = gTextureMaps[gObjectCB.MatIndex].Sample(gsamLinearWrap, input.UV);
    
    if (color.a < 0.9f)
    {
        discard;
    }
    
    if (input.UV.x > gObjectCB.SliderValue)
    {
        discard;
    }

    return color;
}
