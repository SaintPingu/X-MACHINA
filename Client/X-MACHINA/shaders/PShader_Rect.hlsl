#include "Light.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSScreenRectSamplingTextured(VSOutput_Tex input) : SV_TARGET
{
    // UI는 Forward 쉐이더로 바로 후면 버퍼에 렌더링한다.
    //float4 uiColor = gTextureMap[gRT2_UIIndex].Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    //if (!IsWhite(uiColor))
    //{
    //    return uiColor;
    //}
    
    float4 color = gTextureMap[gRT1_TextureIndex].Load(uint3((uint) input.PosH.x, (uint) input.PosH.y, 0));
    float4 white = float4(1.f, 1.f, 1.f, 1.f);
    if (IsWhite(color))
    {
        if (color.r == 1.f && color.g < 1.f && color.b < 1.f) // [temp] for bounding box
        {
            return color;
        }
        discard;
    }
    
    float distance = gTextureMap[gRT4_DistanceIndex].Load(uint3((uint) input.PosH.x, (uint) input.PosH.y, 0));
    color = FogDistance(color, distance);

    return color;
}
