#include "Light.hlsl"

struct VSOutput_Tex {
    float4 Position : SV_POSITION;
    float2 UV : UV;
};

float4 PSScreenRectSamplingTextured(VSOutput_Tex input) : SV_Target
{
    float4 uiColor = gTextureMap[gRT2_UIIndex].Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    if (!IsWhite(uiColor))
    {
        return uiColor;
    }
    
    float4 color = gTextureMap[gRT1_TextureIndex].Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    float4 white = float4(1.f, 1.f, 1.f, 1.f);
    if (IsWhite(color))
    {
        if (color.r == 1.f && color.g < 1.f && color.b < 1.f) // [temp] for bounding box
        {
            return color;
        }
        discard;
    }
    
    float distance = gTextureMap[gRT4_DistanceIndex].Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    color = FogDistance(color, distance);

    return color;
}
