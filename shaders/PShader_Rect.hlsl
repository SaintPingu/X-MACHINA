#include "Light.hlsl"

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

float4 PSScreenRectSamplingTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 uiColor = rt2_ui.Load(uint3((uint) input.position.x, (uint) input.position.y, 0));
    if (!IsWhite(uiColor))
    {
        return uiColor;
    }
    
    float4 color = rt1_texture.Load(uint3((uint) input.position.x, (uint) input.position.y, 0));
    float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (IsWhite(color))
    {
        if (color.r == 1.0f && color.g < 1.0f && color.b < 1.0f) // [temp] for bounding box
        {
            return color;
        }
        discard;
    }
    
    float distance = rt4_distance.Load(uint3((uint) input.position.x, (uint) input.position.y, 0));
    color = FogDistance(color, distance);

    return (color);
}
