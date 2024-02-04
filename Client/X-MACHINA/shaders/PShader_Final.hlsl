#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSFinal(VSOutput_Tex pin) : SV_TARGET
{
    // deferred shader에서 GammaDecoding을 진행하기 때문에 최종 출력 쉐이더에서는 GammaEncoding을 해줘야 한다.
    float4 color = gTextureMap[gPassCB.RT0_TextureIndex].Sample(gsamAnisotropicWrap, pin.UV);
    float4 distance = gTextureMap[gPassCB.RT4_DistanceIndex].Sample(gsamAnisotropicWrap, pin.UV);
    //color = FogDistance(color, distance);

    return color;
}
