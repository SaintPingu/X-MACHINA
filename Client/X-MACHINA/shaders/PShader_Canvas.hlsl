#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    // ��Ƽ������ ������� �ʴ� ��� MaterialIndex�� �ٷ� �ؽ�ó �ε����� Set�� ��
    float4 color = gTextureMap[gObjectCB.MatIndex].Sample(gSamplerState, input.UV);
    if (color.a < 0.9f)
    {
        discard;
    }

    return color;
}
