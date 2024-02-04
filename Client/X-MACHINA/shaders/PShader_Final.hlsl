#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSFinal(VSOutput_Tex pin) : SV_TARGET
{
    // deferred shader���� GammaDecoding�� �����ϱ� ������ ���� ��� ���̴������� GammaEncoding�� ����� �Ѵ�.
    float4 color = gTextureMap[gPassCB.RT0_TextureIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 distance = gTextureMap[gPassCB.RT4_DistanceIndex].Sample(gsamAnisotropicWrap, pin.UV);
    //color = FogDistance(color, distance);

    return color;
}
