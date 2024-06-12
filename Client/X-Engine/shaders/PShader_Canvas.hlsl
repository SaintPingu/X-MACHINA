#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_TARGET
{
    // 머티리얼을 사용하지 않는 경우 MaterialIndex에 바로 텍스처 인덱스를 Set할 것
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
