#include "PSResource.hlsl"

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

float4 PSCanvas(VS_TEXTURED_OUTPUT input) : SV_Target2
{
    float4 color = albedoTexture.Sample(samplerState, input.uv);
    if (color.a < 0.9f)
    {
        discard;
    }

    return (color);
}
