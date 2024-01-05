#include "Light.hlsl"

struct VSOutput_Tex {
    float4 Position : SV_POSITION;
    float2 UV : UV;
};

float4 PSScreenRectSamplingTextured(VSOutput_Tex input) : SV_Target
{
    float4 uiColor = gRT2_UI.Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    if (!IsWhite(uiColor))
    {
        return uiColor;
    }
    
    float4 color = gRT1_Texture.Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    float4 white = float4(1.f, 1.f, 1.f, 1.f);
    if (IsWhite(color))
    {
        if (color.r == 1.f && color.g < 1.f && color.b < 1.f) // [temp] for bounding box
        {
            return color;
        }
        discard;
    }
    
    float distance = gRT4_Distance.Load(uint3((uint) input.Position.x, (uint) input.Position.y, 0));
    color = FogDistance(color, distance);

    return color;
}
