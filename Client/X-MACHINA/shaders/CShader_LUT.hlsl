#include "Common.hlsl"

cbuffer BlurInfo : register(b0)
{
	float gTotalTime;
    int gFilterOption;
};

Texture2D gInput				  : register(t0);
Texture2D gLUTTexture0		      : register(t1);
Texture2D gLUTTexture1		      : register(t2);
RWTexture2D<float4> gOutput		  : register(u0);

static const float3x3 aces_input_matrix =
{
    float3(0.59719f, 0.35458f, 0.04823f),
    float3(0.07600f, 0.90834f, 0.01566f),
    float3(0.02840f, 0.13383f, 0.83777f)
};

static const float3x3 aces_output_matrix =
{
    float3(1.60475f, -0.53108f, -0.07367f),
    float3(-0.10208f, 1.10813f, -0.00605f),
    float3(-0.00327f, -0.07276f, 1.07602f)
};

float3 rtt_and_odt_fit(float3 color)
{
    float3 a = color * (color + 0.0245786f) - 0.000090537f;
    float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 aces_fitted(float3 color)
{
    color = mul(aces_input_matrix, color);
    color = rtt_and_odt_fit(color);
    return mul(aces_output_matrix, color);
}


[numthreads(256, 1, 1)]
void LUTCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    float4 color = gInput[float2(dispatchThreadID.x, dispatchThreadID.y)];
 
    // 톤매핑
    if (gFilterOption & Filter_Tone)
    {
        color = GammaDecoding(color);
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        //color = GammaEncoding(saturate((color * (a * color + b)) / (color * (c * color + d) + e)));
        color = GammaEncoding(float4(aces_fitted(color.rgb), color.a));
    }
    
    if (gFilterOption & Filter_LUT)
    {
        float u = floor(color.b * 15.f) / 15.f * 240.f;
        u = (floor(color.r * 15.f) / 15.f * 15.f) + u;
        float v = floor(color.g * 15.f);
        float3 leftColor0 = gLUTTexture0[float2(u, v)].rgb;
        float3 leftColor1 = gLUTTexture1[float2(u, v)].rgb;
        
        u = ceil(color.b * 15.f) / 15.f * 240.f;
        u = (ceil(color.r * 15.f) / 15.f * 15.f) + u;
        v = ceil(color.g * 15.f);
        float3 rightColor0 = gLUTTexture0[float2(u, v)].rgb;
        float3 rightColor1 = gLUTTexture1[float2(u, v)].rgb;
        
        // 태양의 위치와 방향에 따라 lerp에 값을 넣어줘야 한다.
        float3 leftColor = lerp(leftColor0, leftColor1, cos(gTotalTime) * 0.5f + 0.5f);
        float3 rightColor = lerp(rightColor0, rightColor1, cos(gTotalTime) * 0.5f + 0.5f);
        
        color = float4(lerp(leftColor, rightColor, frac(color.rgb * 15.f)), color.a);
    }
    
    gOutput[dispatchThreadID.xy] = color;
}
