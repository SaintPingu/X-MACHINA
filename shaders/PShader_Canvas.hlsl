#include "PSResource.hlsl"

struct VSOutput_Tex {
    float4 Position : SV_POSITION;
    float2 UV : UV;
};

float4 PSCanvas(VSOutput_Tex input) : SV_Target2
{
    float4 color = gAlbedoTexture.Sample(gSamplerState, input.UV);
    if (color.a < 0.9f)
    {
        discard;
    }

    return color;
}
